/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    driver_lcd_ls0x
 * @ingroup     drivers
 * @brief       Generic driver for Sharp LS0x Memory LCDs
 *
 * @{
 * @file
 * @brief       Driver interface for Sharp LS0x Memory LCDs
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef LCD_LS0X_H
#define LCD_LS0X_H

#include <stdint.h>

#include "periph/gpio.h"
#include "periph/spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Part numbers for members of the LS0x LCD family
 */
enum {
    LCD_LS013B7DH01,
    LCD_LS013B7DH03,
    LCD_LS013B4DN01,
    LCD_LS013B4DN02,
    LCD_LS027B7DH01,
    LCD_LS044Q4DH01,
} ls0x_part_t;

/**
 * @brief   LS0x LCD device descriptor
 */
typedef struct {
    ls0x_part_t model;  /**< precise model of the device family */
    spi_t spi;          /**< SPI bus connected to the display */
    gpio_t scs;         /**< chip select pin */
    gpio_t extcomin;    /**< COM inversion pin */
    gpio_t disp;        /**< display pin (on: high, off: low */
} ls0x_t;

/**
 * @brief   The parameter strcut and the device descriptor are identical for
 *          this driver
 */
typedef ls0x_t ls0x_params_t;


void ls0x_setup(ls0x_params_t *params, ls0x_part_t model, spi_t spi,
               gpio_t scs, gpio_t extcomin, gpio_t disp);

int ls0x_init(ls0x_t *dev, ls0x_params_t *params);

void ls0x_write_line(ls0x_t *dev, uint8_t row, uint8_t *buf, size_t len);









#ifdef __cplusplus
}
#endif

#endif /* LCD_LS0X_H */
