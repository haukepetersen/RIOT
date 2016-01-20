/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   drivers_isl29020
 * @{
 *
 * @file
 * @brief     Default configuration for ISL29020 devices
 *
 * @author    Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef ISL29020_PARAMS_H
#define ISL29020_PARAMS_H

#include "board.h"
#include "isl29020.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default parameters for ISL29020 devices
 * @{
 */
#ifndef ISL29020_PARAM_I2C
#define ISL29020_PARAM_I2C          (I2C_DEV(0))
#endif
#ifndef ISL29020_PARAM_ADDR
#define ISL29020_PARAM_ADDR         (ISL29020_DEFAULT_ADDRESS)
#endif
#ifndef ISL29020_PARAM_RANGE
#define ISL29020_PARAM_RANGE        (ISL29020_RANGE_16K)
#endif
#ifndef ISL29020_PARAM_MODE
#define ISL29020_PARAM_MODE         (ISL29020_MODE_AMBIENT)
#endif

#define ISL29020_PARAMS_DEFAULT     {.i2c = ISL29020_PARAM_I2C, \
                                     .addr = ISL29020_PARAM_ADDR, \
                                     .range = ISL29020_PARAM_RANGE, \
                                     .mode = ISL29020_PARAM_MODE}
/** @} */

/**
 * @brief   ISL29020 configuration
 */
static const  isl29020_params_t isl29020_params[] =
{
#ifdef ISL29020_PARAMS_BOARD
    ISL29020_PARAMS_BOARD
#else
    ISL29020_PARAMS_DEFAULT
#endif
};

/**
 * @brief   Get number of defined devices
 */
#define ISL29020_NUMOF  (sizeof(isl29020_params) / sizeof(isl29020_params[0]))

/**
 * @brief   Entries to the SAUL registry
 */
saul_reg_t isl29020_saul[] =
{
    {
        .name = "isl29020",
        .driver = &isl29020_saul_driver
    }
};

#ifdef __cplusplus
}
#endif

#endif /* ISL29020_PARAMS_H */
/** @} */
