/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_vl53l0x
 *
 * @{
 * @file
 * @brief       Default configuration for VL53L0X devices
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */


#ifndef VL53L0X_PARAMS_H
#define VL53L0X_PARAMS_H

#include "board.h"
#include "vl53l0x.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for VL53L0X devices
 * @{
 */
#ifndef VL53L0X_PARAM_I2C
#define VL53L0X_PARAM_I2C       I2C_DEV(0)
#endif
#ifndef VL53L0X_PARAMS_ADDR
#define VL53L0X_PARAMS_ADDR     VL53L0X_I2C_ADDR_DFLT
#endif

#ifndef VL53L0X_PARAMS
#define VL53L0X_PARAMS          { .bus = VL53L0X_PARAM_I2C, \
                                  .addr = VL53L0X_PARAMS_ADDR }
#endif
/**@}*/

/**
 * @brief   VL53L0X configuration
 */
static const vl53l0x_params_t vl53l0x_params[] =
{
    VL53L0X_PARAMS
};


#ifdef __cplusplus
}
#endif

#endif /* VL53L0X_PARAMS_H */
/** @} */
