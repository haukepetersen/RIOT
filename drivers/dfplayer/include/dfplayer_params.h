/*
 * Copyright (C) 2018 Hauke Petersen <devel@haukepetersen.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_dfplayer
 *
 * @{
 * @file
 * @brief       Default configuration for DFPlayer Mini devices
 *
 * @author      Hauke Petersen <devel@haukepetersen.de>
 */

#ifndef DFPLAYER_PARAMS_H
#define DFPLAYER_PARAMS_H

#include "board.h"
#include "dfplayer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for DFPLAYER devices
 * @{
 */
#ifndef DFPLAYER_PARAM_UART
#define DFPLAYER_PARAM_UART         UART_DEV(1)
#endif
#ifndef DFPLAYER_PARAM_BAUDRATE
#define DFPLAYER_PARAM_BAUDRATE     (DFPLAYER_BAUDRATE_DEFAULT)
#endif

#ifndef DFPLAYER_PARAMS
#define DFPLAYER_PARAMS             { .uart     = DFPLAYER_PARAM_UART,   \
                                      .baudrate = DFPLAYER_PARAM_BAUDRATE }
#endif
/**@}*/

/**
 * @brief   DFPlayer configuration
 */
static const dfplayer_params_t dfplayer_params[] =
{
    DFPLAYER_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* DFPLAYER_PARAMS_H */
/** @} */

