/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     driver_lcd_ls0x
 * @{
 *
 * @file
 * @brief       Sharp LS0x Memory LCD driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdint.h>

#include "lcd_ls0x.h"

/**
 * @brief   Use a 1MHz SPI speed for this device
 */
#define LS0X_SPI_SPEED      (SPI_SPEED_1MHZ)

/**
 * @brief   The LCD takes data on the first rising edge of the clock signal
 */
#define LS0X_SPI_MODE       (SPI_CONF_FIRST_RISING)

/**
 * @brief   Temporary definition until re-worked SPI driver is merged
 */
#define SPI_UNDEF           -1

/**
 * @brief   Device model configuration struct
 */
typedef struct {
    uint8_t w;          /**< height (pixel) */
    uint8_t h;          /**< width (pixel) */
    uint8_t u;          /**< voltage */
} ls0x_conf_t;

/**
 * @brief   Static device configuration for different models
 */
static const ls0x_conf_t ls0x_conf[] = {
    [LCD_LS013B7DH01] = {144, 168, 5},
    [LCD_LS013B7DH03] = {128, 128, 3},
    [LCD_LS013B4DN01] = {96, 96, 5},
    [LCD_LS013B4DN02] = {96, 96, 5},
    [LCD_LS027B7DH01] = {400, 240, 5},
    [LCD_LS044Q4DH01] = {320, 240, 5},
}

void ls0x_setup(ls0x_params_t *params, ls0x_part_t model, spi_t spi,
               gpio_t scs, gpio_t extcomin, gpio_t disp);
{
    params->model = model;
    params->spi = spi;
    params->scs = scs;
    params->extcomin = extcomin;
    params->disp = disp;
}

int ls0x_init(ls0x_t *dev, ls0x_params_t *params)
{
    /* check parameters */
    if ((params->spi == SPI_UNDEF) || (params->scs == GPIO_UNDEF) ||
        (params->extcomin == GPIO_UNDEF)) {
        return -1;
    }
    if ((void *)dev != (void *)params) {
        memcpy(dev, params, sizeof(ls0x_t));
    }

    /* initialize SPI bus */
    spi_init(dev->spi, LS0X_SPI_MODE, LS0X_SPI_SPEED);
    /* initialize pins */
    gpio_init(dev->scs, GPIO_DIR_OUT, GPIO_NOPULL);
    gpio_set(dev->scs);
    gpio_init(dev->extcomin, GPIO_DIR_OUT, GPIO_NOPULL);
    if (dev->disp != GPIO_UNDEF) {
        gpio_init(dev->disp)
    }
}

void ls0x_write_line(ls0x_t *dev, uint8_t row, uint8_t *buf, size_t len)
{
    /* check some parameters */
    if (((uint8_t)len != ls0x_conf[dev->model].w) ||
        (row > ls0x_conf[dev->model])) {
        return;
    }

    gpio_clear(dev->scs);
    spi_transfer_reg(dev->spi, CMD, (char)row, NULL);
    spi_transfer_bytes(dev->spi, buf, NULL, len);
    gpio_set(dev->scs);
}
