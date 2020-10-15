/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_ds3231 DS3231 Real Time Clock
 * @ingroup     drivers_sensors
 * @brief       Driver for the Maxim DS3231 extremely accurate RTC
 *
 * @{
 * @file
 * @brief       Interface definition for the Maxim DS3231 RTC
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef DS3231_H
#define DS3231_H

#include <errno.h>

#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default address of DS3231 sensors
 */
#define DS3231_I2C_ADDR             0x68

/**
 * @brief   Configuration options
 */
enum {
    DS3231_POWERON_ON_INIT = 0x01,     /* power on device on init */
};

// note: called in interrupt context
typedef void(ds3231_cb_t)(void *);

typedef enum {
    DS3231_EVERY_SEC            = 0x00f,
    DS3231_MATCH_SEC            = 0x00e,
    DS3231_MATCH_MIN_SEC        = 0x00c,
    DS3231_MATCH_H_MIN_SEC      = 0x008,
    DS3231_MATCH_DATE_H_MIN_SEC = 0x000,
    DS3231_MATCH_DAY_H_MIN_SEC  = 0x010,
    DS3231_EVERY_MIN            = 0x107,
    DS3231_MATCH_MIN            = 0x106,
    DS3231_MATCH_H_MIN          = 0x104,
    DS3231_MATCH_DATE_H_MIN     = 0x100,
    DS3231_MATCH_DAY_H_MIN      = 0x110,
} ds3231_alarm_cfg_t;

/**
 * @brief   Device descriptor for DS3231 devices
 */
typedef struct {
    i2c_t bus;          /**< I2C bus the device is connected to */
    gpio_t int_sqw;     /**< interrupt / square wave pin */
    ds3231_cb_t cb;     /**< alarm callback function */
    void *arg;          /**< callback argument */
} ds3231_t;

/**
 * @brief   Set of configuration parameters for DS3231 devices
 */
typedef struct {
    i2c_t bus;          /**< I2C bus the device is connected to */
    uint8_t opt;        /**< additional options */
    gpio_t int_sqw;
} ds3231_params_t;


/**
 * @brief   Initialize the given DS3231 device
 *
 * @param[out] dev      device descriptor of the targeted device
 * @param[in] params    device configuration (i2c bus, address and bus clock)
 *
 * @return      0 on success
 * @return      -ENODEV if no DS3231 device was found on the bus
 */
int ds3231_init(ds3231_t *dev, const ds3231_params_t *params);


int ds3231_get_time(const ds3231_t *dev, struct tm *time);


int ds3231_set_time(const ds3231_t *dev, const struct tm *time);

int ds3231_set_alarm(const ds3231_t *dev, ds3231_alarm_type_t type,
                     const struct tm *time);

int ds3231_check_alarm(const ds3231_t *dev);

int ds3231_clear_alarm(const ds3231_t *dev);


int ds3231_poweroff(const ds3231_t *dev);


int ds3231_poweron(const ds3231_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* DS3231_H */
/** @} */
