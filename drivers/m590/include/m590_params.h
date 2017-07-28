/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_m590
 *
 * @{
 * @file
 * @brief       Default configuration for M590 GSM modems
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef M590_PARAMS_H
#define M590_PARAMS_H

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name   Set default configuration parameters for the M590 modems
 * @{
 */
#ifndef M590_PARAM_UART
#define M590_PARAM_UART         (UART_DEV(0))
#endif
#ifndef M590_PARAM_BAUDRATE
#define M590_PARAM_BAUDRATE     (115200U)
#endif

#define M590_PARAMS_DEFAULT     { .uart     = M590_PARAM_UART, \
                                  .baudrate = M590_PARAM_BAUDRATE }
/**@}*/

/**
 * @brief   Allocation of M590 configuration
 */
static const m590_params_t m590_params[] =
{
#ifdef M590_PARAMS_BOARD
    M590_PARAMS_BOARD,
#else
    M590_PARAMS_DEFAULT,
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* M590_PARAMS_H */
/** @} */
