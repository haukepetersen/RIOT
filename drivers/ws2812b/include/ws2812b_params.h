/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ws2812b
 * @{
 *
 * @file
 * @brief       Default parameters for WS2812B based RGB LEDs
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef WS2812B_PARAMS_H
#define WS2812B_PARAMS_H

#include "board.h"
#include "ws2812b.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name   Set default configuration parameters for WS2812B LEDs
 * @{
 */
#ifndef WS2812B_PARAM_PIN
#define WS2812B_PARAM_PIN       GPIO_PIN(0, 7)
#endif
#ifndef WS2812B_PARAM_NUMOF
#define WS2812B_PARAM_NUMOF     (1)
#endif

#define WS2812B_PARAMS_DEFAULT  { .pin   = WS2812B_PARAM_PIN,  \
                                  .numof = WS2812B_PARAM_NUMOF }
/**@}*/

/**
 * @brief   Configure WS2812B LEDs
 */
static const ws2812b_params_t ws2812b_params[] =
{
#ifdef WS2812B_PARAMS_CUSTOM
    WS2812B_PARAMS_CUSTOM,
#else
    WS2812B_PARAMS_DEFAULT,
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* WS2812B_PARAMS_H */
/** @} */
