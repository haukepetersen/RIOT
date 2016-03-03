/*
 * Copyright (C) 2014 FU Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  driver_periph
 * @{
 *
 * @file
 * @brief       Low-level I2C driver implementation
 *
 * @note This implementation only implements the 7-bit addressing mode.
 *
 * For implementation details please refer to STM application note AN2824.
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Víctor Ariño <victor.arino@triagnosys.com>
 *
 * @}
 */

#include <stdint.h>

#include "cpu.h"
#include "irq.h"
#include "mutex.h"
#include "periph_conf.h"
#include "periph/i2c.h"
#include "periph/gpio.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

/* static function definitions */
static void _i2c_init(I2C_TypeDef *i2c, int ccr);
static void _pin_config(gpio_t pin_scl, gpio_t pin_sda);
static void _start(I2C_TypeDef *dev, uint8_t address, uint8_t rw_flag, char *err);
static inline void _clear_addr(I2C_TypeDef *dev);
static inline void _write(I2C_TypeDef *dev, char *data, int length, char *err);
static inline void _stop(I2C_TypeDef *dev, char *err);

/**
 * @brief Array holding one pre-initialized mutex for each I2C device
 */
static mutex_t locks[] =  {
#if I2C_0_EN
    [I2C_0] = MUTEX_INIT,
#endif
#if I2C_1_EN
    [I2C_1] = MUTEX_INIT,
#endif
#if I2C_2_EN
    [I2C_2] = MUTEX_INIT
#endif
#if I2C_3_EN
    [I2C_3] = MUTEX_INIT
#endif
};

static char err_flag[] = {
#if I2C_0_EN
    [I2C_0] = 0x00,
#endif
#if I2C_1_EN
    [I2C_1] = 0x00,
#endif
#if I2C_2_EN
    [I2C_2] = 0x00
#endif
#if I2C_3_EN
    [I2C_3] = 0x00
#endif
};

/**
 * @brief data structure for saving the bus state
 */
typedef struct {
    mutex_t lock;
} i2c_state_t;

typedef struct {
    I2C_TypeDef *dev;
    DMA_TypeDef *dma;
    DMA_Channel_TypeDef *dma_rx;
    DMA_Channel_TypeDef *dma_tx;
} i2c_conf_t;

/**
 * @brief static mapping of RIOT I2C devices to hardware I2C devices
 */
static const i2c_conf_t i2c_conf[] = {
#if I2C_0_EN
    {I2C_0_DEV, I2C_0_DMA_DEV, I2C_0_DMA_RX_CH, I2C_0_DMA_TX_CH}
#endif
};

int i2c_init_master(i2c_t dev, i2c_speed_t speed)
{
    I2C_TypeDef *i2c;
    gpio_t pin_scl, pin_sda;
    int ccr;

    /* read speed configuration */
    switch (speed) {
        case I2C_SPEED_NORMAL:
            ccr = I2C_APBCLK / 200000;
            break;
        case I2C_SPEED_FAST:
            ccr = I2C_APBCLK / 800000;
            break;
        default:
            return -2;
    }

    /* read static device configuration */
    switch (dev) {
#if I2C_0_EN
        case I2C_0:
            i2c = I2C_0_DEV;
            pin_scl = I2C_0_SCL_PIN;
            pin_sda = I2C_0_SDA_PIN;
            I2C_0_CLKEN();
            I2C_0_DMA_CLKEN();

            NVIC_EnableIRQ(I2C_0_DMA_RX_IRQ);
            NVIC_EnableIRQ(I2C_0_DMA_TX_IRQ);
            NVIC_EnableIRQ(I2C_0_ERR_IRQ);

            break;
#endif
#if I2C_1_EN
        case I2C_1:
            i2c = I2C_1_DEV;
            pin_scl = I2C_1_SCL_PIN;
            pin_sda = I2C_1_SDA_PIN;
            I2C_1_CLKEN();
            NVIC_SetPriority(I2C_1_ERR_IRQ, I2C_IRQ_PRIO);
            NVIC_EnableIRQ(I2C_1_ERR_IRQ);
            break;
#endif
        default:
            return -1;
    }

    /* disable peripheral */
    i2c->CR1 &= ~I2C_CR1_PE;

    /* initialize state variables */
    mutex_init(&(i2c_state[dev].lock));
    mutex_lock(&(i2c_state[dev].lock));

    /* configure DMA RX channel: highest priority, 8-bit data,
       memory increment, write to memory, transfer complete interrupt enable */
    i2c_conf[dev].dma_rx->CCR = (DMA_CCR1_PL | DMA_CCR1_MINC | DMA_CCR1_TCIE);
    i2c_conf[dev].dma_rx->CPAR = i2c_conf[dev].dev->DR;
    /* configure DMA TX channel: highest priority, 8-bit data,
       memory increment, write to peripheral, TC interrupt */
    i2c_conf[dev].dma_tx->CCR = (DMA_CCR1_PL | DMA_CCR1_MINC | DMA_CCR1_DIR | DMA_CCR1_TCIE);
    i2c_conf[dev].dma_tx->CPAR = i2c_conf[dev].dev->DR;

    /* disable device */
    i2c_conf[dev].dev->CR1 = 0;
    /* configure I2C clock and enable error interrupt */
    i2c_conf[dev].dev->CR2 = (I2C_APBCLK / 1000000) | I2C_CR2_ITERREN;
    i2c_conf[dev].dev->CCR = ccr;
    i2c_conf[dev].dev->TRISE = (I2C_APBCLK / 1000000) + 1;
    /* configure device */
    i2c_conf[dev].dev->OAR1 = 0;              /* makes sure we are in 7-bit address mode */
    /* enable device */
    i2c_conf[dev].dev->CR1 |= I2C_CR1_PE;

    /* configure pins */
    _pin_config(pin_scl, pin_sda);
    /* configure device */
    _i2c_init(i2c, ccr);

    /* make sure the analog filters don't hang -> see errata sheet 2.14.7 */
    if (i2c->SR2 & I2C_SR2_BUSY) {
        DEBUG("LINE BUSY AFTER RESET -> toggle pins now\n");
        /* disable peripheral */
        i2c->CR1 &= ~I2C_CR1_PE;
        /* re-run pin config to toggle and re-configure pins */
        _pin_config(pin_scl, pin_sda);
        /* make peripheral soft reset */
        i2c->CR1 |= I2C_CR1_SWRST;
        i2c->CR1 &= ~I2C_CR1_SWRST;
        /* enable device */
        _i2c_init(i2c, ccr);
    }

    return 0;
}

static void _i2c_init(I2C_TypeDef *i2c, int ccr)
{
    /* disable device and set ACK bit */
    i2c->CR1 = I2C_CR1_ACK;
    /* configure I2C clock and enable error interrupts */
    i2c->CR2 = (I2C_APBCLK / 1000000) | I2C_CR2_ITERREN;
    i2c->CCR = ccr;
    i2c->TRISE = (I2C_APBCLK / 1000000) + 1;
    /* configure device */
    i2c->OAR1 = 0;              /* makes sure we are in 7-bit address mode */
    /* enable device */
    i2c->CR1 |= I2C_CR1_PE;
}

static void _pin_config(gpio_t scl, gpio_t sda)
{
    /* toggle pins to reset analog filter -> see datasheet */
    /* set as output */
    gpio_init(scl, GPIO_DIR_OUT, GPIO_NOPULL);
    gpio_init(sda, GPIO_DIR_OUT, GPIO_NOPULL);
    /* run through toggling sequence */
    gpio_set(scl);
    gpio_set(sda);
    gpio_clear(sda);
    gpio_clear(scl);
    gpio_set(scl);
    gpio_set(sda);
    /* configure the pins alternate function */
    gpio_init_af(scl, GPIO_AF_OUT_OD);
    gpio_init_af(sda, GPIO_AF_OUT_OD);
}

int i2c_acquire(i2c_t dev)
{
    if (dev >= I2C_NUMOF) {
        return -1;
    }
    mutex_lock(&locks[dev]);
    return 0;
}

int i2c_release(i2c_t dev)
{
    if (dev >= I2C_NUMOF) {
        return -1;
    }
    mutex_unlock(&locks[dev]);
    return 0;
}

int i2c_read_byte(i2c_t dev, uint8_t address, char *data)
{
    return i2c_read_bytes(dev, address, data, 1);
}

int i2c_read_bytes(i2c_t dev, uint8_t address, char *data, int length)
{
    if (dev >= I2C_NUMOF) {
        return -1;
    }

    /* configure and enable DMA channel */
    DEBUG("Setting RX DMA channel\n");
    i2c_conf[dev].dma_rx->CNDTR = length;
    i2c_conf[dev].dma_rx->CMAR = (uint32_t)data;
    i2c_conf[dev].dma_rx->CCR |= DMA_CCR1_EN;
    /* enable DMA and set LAST bit */
    DEBUG("enable DMA and LAST bit\n");
    i2c_conf[dev].dev->CR2 |= (I2C_CR2_DMAEN | I2C_CR2_LAST);
    /* send start condition */
    _start(i2c_conf[dev].dev, address, I2C_FLAG_READ);
    /* wait for transfer to be complete */
    DEBUG("Locking mutex\n");
    mutex_lock(&(i2c_state[dev].lock));
    DEBUG("mutex unlocked\n");
    /* program STOP bit */
    _stop(i2c_conf[dev].dev);
    return length;
}

int i2c_read_reg(i2c_t dev, uint8_t address, uint8_t reg, char *data)
{
    DEBUG("BLAH\n");
    return i2c_read_regs(dev, address, reg, data, 1);

}

int i2c_read_regs(i2c_t dev, uint8_t address, uint8_t reg, char *data, int length)
{
    DEBUG("FOO\n");
    if (dev >= I2C_NUMOF) {
        return -1;
    }

    /* send start condition and slave address */
    DEBUG("Send slave address and clear ADDR flag\n");
    _start(i2c_conf[dev].dev, address, I2C_FLAG_WRITE);
    DEBUG("Write reg into DR\n");
    i2c_conf[dev].dev->DR = reg;
    _stop(i2c_conf[dev].dev);
    DEBUG("Now start a read transaction\n");
    return i2c_read_bytes(dev, address, data, length);
}

int i2c_write_byte(i2c_t dev, uint8_t address, char data)
{
    return i2c_write_bytes(dev, address, &data, 1);
}

int i2c_write_bytes(i2c_t dev, uint8_t address, char *data, int length)
{
    if (dev >= I2C_NUMOF) {
        return -1;
    }

    /* configure DMA TX channel */
    DEBUG("Configuring TX DMA channel\n");
    i2c_conf[dev].dma_tx->CNDTR = length;
    i2c_conf[dev].dma_tx->CMAR = (uint32_t)data;
    i2c_conf[dev].dma_tx->CCR |= DMA_CCR1_EN;
    /* enable DMA */
    DEBUG("Enable DMA in I2C module\n");
    i2c_conf[dev].dev->CR2 &= ~I2C_CR2_LAST;
    i2c_conf[dev].dev->CR2 |= I2C_CR2_DMAEN;
    /* start transmission and send slave address */
    _start(i2c_conf[dev].dev, address, I2C_FLAG_WRITE);
    /* wait on mutex for transfer to be finished */
    DEBUG("lock mutex\n");
    mutex_lock(&(i2c_state[dev].lock));
    DEBUG("mutex: unlocked\n");
    /* finish transfer */
    _stop(i2c_conf[dev].dev);
    /* return number of bytes send */
    return length;
}

int i2c_write_reg(i2c_t dev, uint8_t address, uint8_t reg, char data)
{
    return i2c_write_regs(dev, address, reg, &data, 1);
}

int i2c_write_regs(i2c_t dev, uint8_t address, uint8_t reg, char *data, int length)
{
    if (dev >= I2C_NUMOF) {
        return -1;
    }

    /* disable DMA for now */
    i2c_conf[dev].dev->CR2 &= ~I2C_CR2_DMAEN;
    /* configure DMA TX channel */
    i2c_conf[dev].dma_tx->CNDTR = length;
    i2c_conf[dev].dma_tx->CMAR = (uint32_t)data;
    i2c_conf[dev].dma_tx->CCR |= DMA_CCR1_EN;
    /* start transmission and send slave address */
    _start(i2c_conf[dev].dev, address, I2C_FLAG_WRITE);
    /* send address byte */
    i2c_conf[dev].dev->DR = (uint8_t)address;
    /* enable DMA */
    i2c_conf[dev].dev->CR2 &= ~I2C_CR2_LAST;
    i2c_conf[dev].dev->CR2 |= I2C_CR2_DMAEN;
    /* wait on mutex for transfer to be finished */
    mutex_lock(&(i2c_state[dev].lock));
    /* finish transfer */
    _stop(i2c_conf[dev].dev);
    /* return number of bytes send */
    return length;
}

void i2c_poweron(i2c_t dev)
{
    switch (dev) {
#if I2C_0_EN
        case I2C_0:
            I2C_0_CLKEN();
            break;
#endif
#if I2C_1_EN
        case I2C_1:
            I2C_1_CLKEN();
            break;
#endif
    }
}

void i2c_poweroff(i2c_t dev)
{
    switch (dev) {
#if I2C_0_EN
        case I2C_0:
            while (I2C_0_DEV->SR2 & I2C_SR2_BUSY) ;
            I2C_0_CLKDIS();
            break;
#endif
#if I2C_1_EN
        case I2C_1:
            while (I2C_1_DEV->SR2 & I2C_SR2_BUSY) ;
            I2C_1_CLKDIS();
            break;
#endif
    }
}

static void _start(I2C_TypeDef *dev, uint8_t address, uint8_t rw_flag)
{
    /* wait for bus to be ready */
    DEBUG("Wait for bus to be ready -> BUSY not set\n");
    while (dev->SR2 & I2C_SR2_BUSY);
    /* generate start condition */
    DEBUG("Generate start condition\n");
    dev->CR1 |= I2C_CR1_START;
    DEBUG("Wait for SB flag to be set\n");
    while (!(dev->SR1 & I2C_SR1_SB));
    /* send address and read/write flag */
    DEBUG("Send address\n");
    dev->DR = (address << 1) | rw_flag;
    /* clear ADDR flag by reading first SR1 and then SR2 */
    DEBUG("Wait for ADDR flag to be set\n");
    while (!(dev->SR1 & I2C_SR1_ADDR));
    DEBUG("Clear ADDR flag\n");
    dev->SR1;
    dev->SR2;
}

static inline void _stop(I2C_TypeDef *dev)
{
    /* make sure last byte was send */
    DEBUG("Waiting for last byte to be transferred\n");
    while (!(dev->SR1 & I2C_SR1_BTF));
    /* send STOP condition */
    DEBUG("Sending STOP condition\n");
    dev->CR1 |= I2C_CR1_STOP;
    /* wait until stop is cleared by hardware */
    DEBUG("Waiting for STOP bit to be cleared\n");
    while (dev->SR1 & I2C_CR1_STOP);
}

static inline void i2c_irq_handler(i2c_t i2c_dev, I2C_TypeDef *dev)
{
    unsigned volatile state = dev->SR1;

    /* record and clear errors */
    err_flag[i2c_dev] = (state >> 8);
    dev->SR1 &= 0x00ff;

    DEBUG("\n\n### I2C %d ERROR OCCURED ###\n", i2c_dev);
    DEBUG("status: %08x\n", state);
    if (state & I2C_SR1_OVR) {
        DEBUG("OVR\n");
    }
    if (state & I2C_SR1_AF) {
        DEBUG("AF\n");
    }
    if (state & I2C_SR1_ARLO) {
        DEBUG("ARLO\n");
    }
    if (state & I2C_SR1_BERR) {
        DEBUG("BERR\n");
    }
    if (state & I2C_SR1_PECERR) {
        DEBUG("PECERR\n");
    }
    if (state & I2C_SR1_TIMEOUT) {
        DEBUG("TIMEOUT\n");
    }
    if (state & I2C_SR1_SMBALERT) {
        DEBUG("SMBALERT\n");
    }
}

#if I2C_0_EN
void I2C_0_ERR_ISR(void)
{
    i2c_irq_handler(I2C_0, I2C_0_DEV);
}

void I2C_0_DMA_RX_ISR(void)
{
    printf("RX_ISR %08x\n", (unsigned int)I2C_0_DMA_DEV->ISR);
    /* disable DMA channel */
    I2C_0_DMA_RX_CH->CCR &= ~(DMA_CCR1_EN);
    /* clear interrupt flag */
    I2C_0_DMA_DEV->IFCR |= (0xf << I2C_0_DMA_RX_OFF);
    /* wake-up thread */
    mutex_unlock(&(i2c_state[I2C_0].lock));
}

void I2C_0_DMA_TX_ISR(void)
{
    printf("TX_ISR %08x\n", (unsigned int)I2C_0_DMA_DEV->ISR);
    /* disable DMA channel */
    I2C_0_DMA_TX_CH->CCR &= ~(DMA_CCR1_EN);
    /* clear interrupt flags */
    I2C_0_DMA_DEV->IFCR |= (0xf << I2C_0_DMA_TX_OFF);
    /* return to thread mode */
    mutex_unlock(&(i2c_state[I2C_0].lock));
}
#endif

#if I2C_1_EN
void I2C_1_ERR_ISR(void)
{
    i2c_irq_handler(I2C_1, I2C_1_DEV);
}
#endif

#endif /* I2C_0_EN || I2C_1_EN */
