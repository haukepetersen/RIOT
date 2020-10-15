/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ds3231
 * @{
 *
 * @file
 * @brief       Default configuration for DS3231 devices
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */
#ifndef DS3231_PARAMS_H
#define DS3231_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Default configuration parameters for the DS3231 driver
 * @{
 */
#ifndef DS3231_PARAM_I2C
#define DS3231_PARAM_I2C        I2C_DEV(0)
#endif
#ifndef DS3231_PARAM_OPT
#define DS3231_PARAM_OPT        (0)
#endif

#ifndef DS3231_PARAMS
#define DS3231_PARAMS           { .bus = DS3231_PARAM_I2C, \
                                  .opt = DS3231_PARAM_OPT, }
#endif
/** @} */

/**
 * @brief   DS3231 configuration
 */
static const ds3231_params_t ds3231_params[] =
{
    DS3231_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* DS3231_PARAMS_H */
/** @} */
