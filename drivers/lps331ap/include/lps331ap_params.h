/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   drivers_lps331ap
 * @{
 *
 * @file
 * @brief     Default configuration for LPS331AP devices
 *
 * @author    Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef LPS331AP_PARAMS_H
#define LPS331AP_PARAMS_H

#include "board.h"
#include "lps331ap.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default parameters for LPS331AP devices
 * @{
 */
#ifndef LPS331AP_PARAM_I2C
#define LPS331AP_PARAM_I2C          (I2C_DEV(0))
#endif
#ifndef LPS331AP_PARAM_ADDR
#define LPS331AP_PARAM_ADDR         (LPS331AP_DEFAULT_ADDRESS)
#endif
#ifndef LPS331AP_PARAM_RATE
#define LPS331AP_PARAM_RATE         (LPS331AP_RATE_7HZ)
#endif

#define LPS331AP_PARAMS_DEFAULT     {.i2c = LPS331AP_PARAM_I2C, \
                                     .addr = LPS331AP_RATE_7HZ, \
                                     .rate = LPS331AP_PARAM_RATE}
/** @} */

/**
 * @brief    LPS331AP configuration
 */
static const  lps331ap_params_t lps331ap_params[] =
{
#ifdef LPS331AP_PARAMS_BOARD
    LPS331AP_PARAMS_BOARD
#else
    LPS331AP_PARAMS_DEFAULT
#endif
};

/**
 * @brief   Get the number of configured LPS331AP devices
 */
#define LPS331AP_NUMOF  (sizeof(lps331ap_params) / sizeof(lps331ap_params[0]))

/**
 * @brief   Additional meta information to keep in the SAUL registry
 */
saul_reg_t lps331ap_saul[] =
{
    {
        .name = "lps331ap",
        .driver = &lps331ap_saul_driver
    }
};

#ifdef __cplusplus
}
#endif

#endif /* LPS331AP_PARAMS_H */
/** @} */
