/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   drivers_lsm303dlhc
 * @{
 *
 * @file
 * @brief     Default configuration for LSM303DLHC devices
 *
 * @author    Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef LSM303DLHC_PARAMS_H
#define LSM303DLHC_PARAMS_H

#include "board.h"
#include "lsm303dlhc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Define default parameters
 * @{
 */
#ifndef LSM303DLHC_PARAM_I2C
#define LSM303DLHC_PARAM_I2C        (I2C_DEV(0))
#endif
#ifndef LSM303DLHC_PARAM_ACC_ADDR
#define LSM303DLHC_PARAM_ACC_ADDR   (LSM303DLHC_ACC_DEFAULT_ADDRESS)
#endif
#ifndef LSM303DLHC_PARAM_ACC_PIN
#define LSM303DLHC_PARAM_ACC_PIN    (GPIO(0,0))
#endif
#ifndef LSM303DLHC_PARAM_ACC_RATE
#define LSM303DLHC_PARAM_ACC_RATE   (LSM303DLHC_ACC_SAMPLE_RATE_10HZ)
#endif
#ifndef LSM303DLHC_PARAM_ACC_SCALE
#define LSM303DLHC_PARAM_ACC_SCALE  (LSM303DLHC_ACC_SCALE_4G)
#endif
#ifndef LSM303DLHC_PARAM_MAG_ADDR
#define LSM303DLHC_PARAM_MAG_ADDR   (LSM303DLHC_MAG_DEFAULT_ADDRESS)
#endif
#ifndef LSM303DLHC_PARAM_MAG_PIN
#define LSM303DLHC_PARAM_MAG_PIN    (GPIO(0,1))
#endif
#ifndef LSM303DLHC_PARAM_MAG_RATE
#define LSM303DLHC_PARAM_MAG_RATE   (LSM303DLHC_MAG_SAMPLE_RATE_15HZ)
#endif
#ifndef LSM303DLHC_PARAM_MAG_GAIN
#define LSM303DLHC_PARAM_MAG_GAIN   (LSM303DLHC_MAG_GAIN_450_400_GAUSS)
#endif

#define LSM303DLHC_PARAMS_DEFAULT   {.i2c = LSM303DLHC_PARAM_I2C, \
                                     .acc_addr = LSM303DLHC_PARAM_ACC_ADDR, \
                                     .acc_pin = LSM303DLHC_PARAM_ACC_PIN, \
                                     .acc_rate = LSM303DLHC_PARAM_ACC_RATE, \
                                     .acc_scale = LSM303DLHC_PARAM_ACC_SCALE, \
                                     .mag_addr = LSM303DLHC_PARAM_MAG_ADDR, \
                                     .mag_pin = LSM303DLHC_PARAM_MAG_PIN, \
                                     .mag_rate = LSM303DLHC_PARAM_MAG_RATE, \
                                     .mag_gain = LSM303DLHC_PARAM_MAG_GAIN}
/** @} */


/**
 * @brief    LSM303DLHC configuration
 */
static const  lsm303dlhc_params_t lsm303dlhc_params[] =
{
#ifdef LSM303DLHC_PARAMS_BOARD
    LSM303DLHC_PARAMS_BOARD
#else
    LSM303DLHC_PARAMS_DEFAULT
#endif
};

/**
 * @brief   Number of configured devices
 */
#define LSM303DLHC_LEN  (sizeof(lsm303dlhc_params)/sizeof(lsm303dlhc_params[0]))


#ifdef MODULE_SAUL_REG
/**
 * @brief   Additional meta information to keep in the SAUL registry
 *
 * @note    The number of entries must be the same than the number of configured
 *          devices
 */
saul_reg_t lsm303dlhc_saul[][LSM303DLHC_SAUL_DEVS] =
{
    {
        {
            .name = "lsm303dlhc-acc",
            .driver = &lsm303dlhc_saul_acc_driver,
        },
        {
            .name = "lsm303dlhc-mag",
            .driver = &lsm303dlhc_saul_mag_driver,
        }
    }
};
#endif

#ifdef __cplusplus
}
#endif

#endif /* LSM303DLHC_PARAMS_H */
/** @} */
