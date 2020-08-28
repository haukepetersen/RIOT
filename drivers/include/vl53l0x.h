/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_vl53l0x VL53L0X ToF Distance Sensor
 * @ingroup     drivers_sensors
 * @brief       Driver for STM VL53L0X time-of-flight distance sensors
 *
 * @{
 * @file
 * @brief       Interface definition for the VL53L0X distance sensor driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef VL53L0X_H
#define VL53L0X_H

#include <stdint.h>

#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   I2C slave address, hard coded for all VL53L0X devices
 */
#define VL53L0X_I2C_ADDR_DFLT       0x52

/**
 * @brief   Return values used by this driver
 */
enum {
    VL53L0X_OK    = 0,
    VL53L0X_NODEV = -1,
    VL53L0X_BUSER = -2,
};

typedef struct {
    i2c_t bus;
    uint8_t addr;
} vl53l0x_params_t;

typedef struct {
    i2c_t bus;
    uint8_t addr;
} vl53l0x_t;


int vl53l0x_init(vl53l0x_t *dev, const vl53l0x_params_t *params);


#ifdef __cplusplus
}
#endif

#endif /* BH1750FVI_H */
/** @} */
