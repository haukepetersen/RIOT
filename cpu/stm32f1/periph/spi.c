/*
 * Copyright (C) 2014-2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_stm32f1
 * @{
 *
 * @file
 * @brief       Low-level SPI driver implementation
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Fabian Nack <nack@inf.fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 *
 * @}
 */

#include "cpu.h"
#include "mutex.h"
#include "periph/gpio.h"
#include "periph/spi.h"
#include "periph_conf.h"
#include "board.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief Array holding one pre-initialized mutex for each SPI device
 */
static mutex_t locks[SPI_NUMOF];

static inline SPI_TypeDef *dev(spi_t bus)
{
    return spi_config[bus].dev;
}

static inline void clk_en(spi_t bus)
{
    if (spi_config[bus].apbbus == APB1) {
        RCC->APB1ENR |= (spi_config[bus].rccmask);
    }
    else {
        RCC->APB2ENR |= (spi_config[bus].rccmask);
    }
}

static inline void clk_dis(spi_t bus)
{
    if (spi_config[bus].apbbus == APB1) {
        RCC->APB1ENR &= ~(spi_config[bus].rccmask);
    }
    else {
        RCC->APB2ENR &= ~(spi_config[bus].rccmask);
    }
}

void spi_init(spi_t bus)
{
    /* make sure given bus is valid */
    assert(bus <= SPI_NUMOF);
    /* initialize the bus lock */
    mutex_init(&locks[bus]);
    /* trigger pin configuration */
    spi_init_pins(bus);
}

void spi_init_pins(spi_t bus)
{
    gpio_init_af(spi_config[bus].pin_clk, GPIO_AF_OUT_PP);
    gpio_init_af(spi_config[bus].pin_mosi, GPIO_AF_OUT_PP);
    gpio_init(spi_config[bus].pin_miso, GPIO_IN);
}

int spi_acquire(spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk)
{
    /* get exclusive bus access */
    mutex_lock(&locks[bus]);
    /* power on the peripheral */
    clk_en(bus);

    /* configure mode and bus clock */
    if ((spi_config[bus].apbbus == APB2) && (clk < 7)) {
        clk += 1;
    }
    dev(bus)->CR1 = (SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR |
                (mode & 0x3) | (clk << 3));
    /* enable the SPI device */
    dev(bus)->CR1 |= SPI_CR1_SPE;

    return SPI_OK;
}

void spi_release(spi_t bus)
{
    /* disable, power off, and release the bus */
    dev(bus)->CR1 = 0;
    clk_dis(bus);
    mutex_unlock(&locks[bus]);
}

void spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out, void *in, size_t len)
{
    uint8_t *out_buf = (uint8_t *)out;
    uint8_t *in_buf  = (uint8_t *)in;

    assert(in || out);

    /* take care of the chip select */
    if (cs != GPIO_UNDEF) {
        gpio_clear((gpio_t)cs);
    }

    if (!in_buf) {
        for (size_t i = 0; i < len; i++) {
            while (!(dev(bus)->SR & SPI_SR_TXE)) {}
            dev(bus)->DR = out_buf[i];
        }
        while ((dev(bus)->SR & SPI_SR_BSY)) {}
        dev(bus)->DR;
    }
    else if (!out_buf) {
        for (size_t i = 0; i < len; i++) {
            dev(bus)->DR = 0;
            while (!dev(bus)->SR & SPI_SR_RXNE) {}
            in_buf[i] = (uint8_t)dev(bus)->DR;
        }
    }
    else {
        for (size_t i = 0; i < len; i++) {
            while (!(dev(bus)->SR & SPI_SR_TXE)) {}
            dev(bus)->DR = out_buf[i];
            while (!(dev(bus)->SR & SPI_SR_RXNE)) {}
            in_buf[i] = (uint8_t)dev(bus)->DR;
        }
    }

    /* finally release chip select line if requested */
    if ((cs != GPIO_UNDEF) && (!cont)) {
        gpio_set((gpio_t)cs);
    }
}
