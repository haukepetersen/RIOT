/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_stm32f0
 * @{
 *
 * @file
 * @brief       Low-level GPIO driver implementation
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 * @author      Hauke Petersen <mail@haukepetersen.de>
 *
 * @}
 */

#include "cpu.h"
#include "board.h"
#include "periph/spi.h"
#include "periph_conf.h"
#include "thread.h"
#include "sched.h"
 #include "stm32f051x8.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

/* guard file in case no SPI device is defined */
#if SPI_NUMOF

/* this value will be send in return of the first transfered byte when in slave mode */
#define RESET_VALUE     (0xbb)

/**
 * @brief unified interrupt handler to be shared between SPI devices
 *
 * @param[in] spi       Pointer to the devices base register
 * @param[in] dev       The device that triggered the interrupt
 */
static inline void irq_handler(SPI_TypeDef *spi, spi_t dev);
static inline void irq_handler_begin(spi_t dev);

/**
 * @brief structure that defines the state for an SPI device
 */
typedef struct {
    char (*cb)(char data);
} spi_state_t;

/**
 * @brief array with one field for each possible SPI device
 */
static spi_state_t spi_config[SPI_NUMOF];


int spi_init_master(spi_t dev, spi_conf_t conf, spi_speed_t speed)
{
    SPI_TypeDef *spi = 0;
    GPIO_TypeDef *port = 0;
    int pin[3];        /* 3 pins: sck, miso, mosi */
    int af;

    /* power on the SPI device */
    spi_poweron(dev);

    switch (dev) {
#if SPI_0_EN
        case SPI_0:
            spi = SPI_0_DEV;
            port = SPI_0_PORT;
            pin[0] = SPI_0_PIN_SCK;
            pin[1] = SPI_0_PIN_MISO;
            pin[2] = SPI_0_PIN_MOSI;
            af = SPI_0_PIN_AF;
            SPI_0_PORT_CLKEN();
            break;
#endif
#if SPI_1_EN
        case SPI_1:
            spi = SPI_1_DEV;
            port = SPI_1_PORT;
            pin[0] = SPI_1_PIN_SCK;
            pin[1] = SPI_1_PIN_MISO;
            pin[2] = SPI_1_PIN_MOSI;
            af = SPI_1_PIN_AF;
            SPI_0_PORT_CLKEN();
            break;
#endif
    }

    /* configure pins for their correct alternate function */
    for (int i = 0; i < 3; i++) {
        port->MODER &= ~(3 << (pin[i] * 2));
        port->MODER |= (2 << (pin[i] * 2));
        int hl = (pin[i] < 8) ? 0 : 1;
        port->AFR[hl] &= (0xf << ((pin[i] - (hl * 8)) * 4));
        port->AFR[hl] |= (af << ((pin[i] - (hl * 8)) * 4));
    }

    /* reset SPI configuration registers */
    spi->CR1 = 0;
    spi->CR2 = 0;
    spi->I2SCFGR = 0;       /* this makes sure SPI mode is selected */

    /* configure bus clock speed */
    switch (speed) {
        case SPI_SPEED_100KHZ:
            spi->CR1 |= (7 << 3);       /* actual clock: 187.5KHz (lowest possible) */
            break;
        case SPI_SPEED_400KHZ:
            spi->CR1 |= (6 << 3);       /* actual clock: 375KHz */
            break;
        case SPI_SPEED_1MHZ:
            spi->CR1 |= (4 << 3);       /* actual clock: 1.5MHz */
            break;
        case SPI_SPEED_5MHZ:
            spi->CR1 |= (2 << 3);       /* actual clock: 6MHz */
            break;
        case SPI_SPEED_10MHZ:
            spi->CR1 |= (1 << 3);       /* actual clock 12MHz */
    }

    /* select clock polarity and clock phase */
    spi->CR1 |= conf;

    /* select master mode */
    spi->CR1 |= SPI_CR1_MSTR;

    /* the NSS (chip select) is managed purely by software */
    //spi->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;
    spi->CR2 |= SPI_CR2_SSOE;

    /* set data-size to 8-bit */
    spi->CR2 |= (7 << 8);

    /* set FIFO threshold to set RXNE when 8 bit are received */
    spi->CR2 |= SPI_CR2_FRXTH;

    /* enable the SPI device */
    spi->CR1 |= SPI_CR1_SPE;

    return 0;
}

int spi_init_slave(spi_t dev, spi_conf_t conf, char (*cb)(char data))
{
    SPI_TypeDef *spi = 0;
    GPIO_TypeDef *port = 0;
    int pin[4];        /* 3 pins: sck, miso, mosi */
    int af;

    /* enable the SPI modules clock */
    spi_poweron(dev);

    switch (dev) {
#if SPI_0_EN
        case SPI_0:
            spi = SPI_0_DEV;
            port = SPI_0_PORT;
            pin[0] = SPI_0_PIN_SCK;
            pin[1] = SPI_0_PIN_MISO;
            pin[2] = SPI_0_PIN_MOSI;
            pin[3] = SPI_0_PIN_NSS;
            af = SPI_0_PIN_AF;
            SPI_0_PORT_CLKEN();

            NVIC_SetPriority(SPI_0_IRQ, SPI_0_IRQ_PRIO);
            NVIC_EnableIRQ(SPI_0_IRQ);

            NVIC_SetPriority(EXTI4_15_IRQn, SPI_0_IRQ_PRIO);
            NVIC_EnableIRQ(EXTI4_15_IRQn);
            /* enable the SYSCFG clock */
            RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
            /* connect PA4 to interrupt. EXTICR[1] (so the second) for pins 4..7, ~0xf = 0000 for PA pin */
            SYSCFG->EXTICR[1] &= ~(0xf);
            EXTI->IMR |= (1 << SPI_0_PIN_NSS);/* 1: interrupt request from line 4 not masked */
            /* configure the active edges */
            EXTI->RTSR &= ~(1 << SPI_0_PIN_NSS); /* 1: rising trigger disabled */
            EXTI->FTSR |= (1 << SPI_0_PIN_NSS); /* 1: falling trigger enabled */
            break;
#endif
#if SPI_1_EN
        case SPI_1:
            spi = SPI_1_DEV;
            port = SPI_1_PORT;
            pin[0] = SPI_1_PIN_SCK;
            pin[1] = SPI_1_PIN_MISO;
            pin[2] = SPI_1_PIN_MOSI;
            af = SPI_1_PIN_AF;
            SPI_1_PORT_CLKEN();
            NVIC_SetPriority(SPI_1_IRQ, SPI_1_IRQ_PRIO);
            NVIC_EnableIRQ(SPI_1_IRQ);
            break;
#endif
    }

    /* set callback */
    spi_config[dev].cb = cb;


    /* configure pins for their correct alternate function */
    for (int i = 0; i < 4; i++) {
        port->MODER &= ~(3 << (pin[i] * 2));
        port->MODER |= (2 << (pin[i] * 2));
        int hl = (pin[i] < 8) ? 0 : 1;
        port->AFR[hl] &= (0xf << ((pin[i] - (hl * 8)) * 4));
        port->AFR[hl] |= (af << ((pin[i] - (hl * 8)) * 4));
    }

    /* reset SPI configuration registers */
    spi->CR1 = 0;
    spi->CR2 = 0;
    spi->I2SCFGR = 0;       /* this makes sure SPI mode is selected */

    /* select clock polarity and clock phase */
    spi->CR1 |= conf;

    /* set data-size to 8-bit */
    spi->CR2 |= (7 << 8);

    /* set FIFO threshold to set RXNE when 8 bit are received */
    spi->CR2 |= SPI_CR2_FRXTH;

    /* enable interrupt for arriving data: 'receive register no empty' and errors */
    spi->CR2 |= SPI_CR2_RXNEIE | SPI_CR2_ERRIE;

    /* enable the SPI device */
    spi->CR1 |= SPI_CR1_SPE;

    return 0;
}

int spi_transfer_byte(spi_t dev, char out, char *in)
{
    //char tmp;
    uint8_t tmp;
    SPI_TypeDef *spi = 0;

    uint32_t spibase;

    DEBUG("Will tranfer char |%c|\n", out);

    switch (dev) {
#if SPI_0_EN
        case SPI_0:
            spi = SPI_0_DEV;
            break;
#endif
#if SPI_1_EN
        case SPI_1:
            spi = SPI_1_DEV;
            break;
#endif
    }


    DEBUG("Wait while TXE is not set\n");
    /* wait for an eventually previous byte to be readily transferred */
    while(!(spi->SR & SPI_SR_TXE));

    DEBUG("Write data into DR\n");
    /* put next byte into the output register */
    spibase = (uint32_t)spi; 
    spibase += 0x0C;
     *(__IO uint8_t *) spibase = out;



    DEBUG("Wait while RXNE is not set\n");
    /* wait until the current byte was successfully transferred */
    while(!(spi->SR & SPI_SR_RXNE) );
  

    DEBUG("Read DR\n");
    /* read response byte to reset flags */
    tmp = *(__IO uint8_t *) spibase;


    /* 'return' response byte if wished for */
    if (in) {
        *in = tmp;
    }

    return 1;
}

int spi_transfer_bytes(spi_t dev, char *out, char *in, unsigned int length)
{
    char res;

    for (int i = 0; i < length; i++) {
        DEBUG("Ready for byte %i\n", i);
        if (out) {
            DEBUG("Send out with real data\n");
            spi_transfer_byte(dev, out[i], &res);
        }
        else {
            DEBUG("Send byte with zero data\n");
            spi_transfer_byte(dev, 0, &res);
        }
        if (in) {
            in[i] = res;
        }
    }

    return length;
}

int spi_transfer_reg(spi_t dev, uint8_t reg, char out, char *in)
{
    spi_transfer_byte(dev, reg, 0);
    return spi_transfer_byte(dev, out, in);
}

int spi_transfer_regs(spi_t dev, uint8_t reg, char *out, char *in, unsigned int length)
{
    spi_transfer_byte(dev, reg, 0);
    return spi_transfer_bytes(dev, out, in, length);
}

void spi_transmission_begin(spi_t dev, char reset_val)
{
    uint32_t spibase;
    switch (dev) {
#if SPI_0_EN
        case SPI_0:
            spibase = (uint32_t)SPI_0_DEV; 
            spibase += 0x0C;
            *(__IO uint8_t *) spibase = reset_val;
            break;
#endif
#if SPI_1_EN
        case SPI_1:
            spibase = (uint32_t)SPI_1_DEV; 
            spibase += 0x0C;
            *(__IO uint8_t *) spibase = reset_val;
            break;
#endif
    }
    DEBUG("SPI: transmisison begins, first char is |%c|\n", reset_val);
}

void spi_poweron(spi_t dev)
{
    switch (dev) {
#if SPI_0_EN
        case SPI_0:
            SPI_0_CLKEN();
            break;
#endif
#if SPI_1_EN
        case SPI_1:
            SPI_1_CLKEN();
            break;
#endif
    }
}

void spi_poweroff(spi_t dev)
{
    switch (dev) {
#if SPI_0_EN
        case SPI_0:
            while (SPI_0_DEV->SR & SPI_SR_BSY);
            SPI_0_CLKDIS();
            break;
#endif
#if SPI_1_EN
        case SPI_1:
            while (SPI_1_DEV->SR & SPI_SR_BSY);
            SPI_1_CLKDIS();
            break;
#endif
    }
}


static inline void irq_handler(SPI_TypeDef *spi, spi_t dev)
{

    char data;
    uint32_t spibase = (uint32_t)spi;
    spibase += 0x0C;
    LD3_TOGGLE;

    /* call owner when new byte was receive (asserts SPI is in slave mode) */
    if (spi->SR & SPI_SR_RXNE) {
        /* read received byte from data register */
        data = *(__IO uint8_t *) spibase;
        /* call callback for receiving the answer of the received byte */
        data = spi_config[dev].cb(data);
        /* set answer byte to be transferred next */
        *(__IO uint8_t *) spibase = data;
    }
    else {
        while (1) {
            for (int i = 0; i < 2000000; i++) {
                asm("nop");
            }
            LD4_TOGGLE;
        }
    }

    /* see if a thread with higher priority wants to run now */
    if (sched_context_switch_request) {
        thread_yield();
    }
}

static inline void irq_handler_begin(spi_t dev)
{
    spi_transmission_begin(dev, RESET_VALUE);
}

#if SPI_0_EN
__attribute__((naked)) void SPI_0_IRQ_HANDLER(void)
{
    ISR_ENTER();
    irq_handler(SPI_0_DEV, SPI_0);
    ISR_EXIT();
}
#endif

#if SPI_1_EN
__attribute__((naked)) void SPI_1_IRQ_HANDLER(void)
{
    ISR_ENTER();
    irq_handler(SPI_1_DEV, SPI_1);
    ISR_EXIT();
}
#endif

#if SPI_0_EN
__attribute__((naked))
void isr_exti4_15(void)
{
    ISR_ENTER();
    if (EXTI->PR & EXTI_PR_PR4) {
        EXTI->PR |= EXTI_PR_PR4;
        irq_handler_begin(SPI_0);
    }
    ISR_EXIT();
}
#endif

#endif /* SPI_NUMOF */
