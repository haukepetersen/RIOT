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

typedef enum {
    DS3231_EVERY_SEC            = 0x0f,
    DS3231_MATCH_SEC            = 0x0e,
    DS3231_MATCH_MIN_SEC        = 0x0c,
    DS3231_MATCH_H_MIN_SEC      = 0x08,
    DS3231_MATCH_DATE_H_MIN_SEC = 0x00,
    DS3231_MATCH_DAY_H_MIN_SEC  = 0x10,
    DS3231_EVERY_MIN            = 0x87,
    DS3231_MATCH_MIN            = 0x86,
    DS3231_MATCH_H_MIN          = 0x84,
    DS3231_MATCH_DATE_H_MIN     = 0x80,
    DS3231_MATCH_DAY_H_MIN      = 0x90,
} ds3231_alarm_type_t;

typedef enum {
    DS3231_SQW_DISABLE          = 0xff,
    DS3231_SQW_1000HZ           = 0x00,
    DS3231_SQW_1024HZ           = 0x08,
    DS3231_SQW_4096HZ           = 0x10,
    DS3231_SQW_8192HZ           = 0x18,
} ds3231_sqw_t;

/**
 * @brief   Device descriptor for DS3231 devices
 */
typedef struct {
    i2c_t bus;          /**< I2C bus the device is connected to */
} ds3231_t;

/**
 * @brief   Set of configuration parameters for DS3231 devices
 */
typedef struct {
    i2c_t bus;          /**< I2C bus the device is connected to */
    uint8_t opt;        /**< additional options */
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

int ds3231_squarewave_config(const ds3231_t *dev, ds3231_sqw_t sqw);


int ds3231_poweroff(const ds3231_t *dev);


int ds3231_poweron(const ds3231_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* DS3231_H */
/** @} */
