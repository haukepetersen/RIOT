/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_ws2812b WS2812B RGB LED driver
 * @ingroup     drivers
 * @brief       Driver for WS2812B based RGB LEDs
 * @{
 *
 * @file
 * @brief       Driver interface for controlling WS2812B based RGB LEDs
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef WS2812B_H
#define WS2812B_H

#include "color.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    WS2812B_OK  =  0,       /**< all ok */
    WS2812B_ERR = -1        /**< internal error */
};

typedef struct {
    gpio_t pin;
} ws2812b_t;

typedef struct {
    gpio_t pin;
    unsigned numof;
} ws2812b_params_t;

int ws2812b_init(ws2812b_t *dev, const ws2812b_params_t *params);

void ws2812b_set(const ws2812b_t *dev, const color_rgb_t *values);

#ifdef __cplusplus
}
#endif

#endif /* WS2812B_H */
/** @} */
