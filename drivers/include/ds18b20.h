/*
 * Copyright (C) 2016-2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_ds18b20 DS18B20
 * @ingroup     drivers
 * @brief       Driver for DS18B20 temperature sensors
 *
 * @{
 * @file
 * @brief       DS18B20 driver interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef DS18B20_H
#define DS18B20_H

#ifdef __cplusplus
extern "C" {
#endif

#include "onewire.h"

enum {
    DS18B20_OK          =  0,
    DS18B20_NODEV       = -1,
    DS18B20_ERR_RES     = -2,
    DS18B20_ERR_CRC     = -3,
};

enum {
    DS18B20_RES_9BIT =  0,          /**< conversion time ~93.75ms */
    DS18B20_RES_10BIT = 1,          /**< conversion time ~187.5ms */
    DS18B20_RES_11BIT = 2,          /**< conversion time ~375ms */
    DS18B20_RES_12BIT = 3,          /**< conversion time ~750ms */
};

typedef struct {
    onewire_rom_t rom;
    uint8_t res;
} ds18b20_params_t;

typedef struct {
    onewire_t *bus;
    onewire_rom_t rom;
    uint32_t t_conv;
} ds18b20_t;

int ds18b20_init(ds18b20_t *dev, onewire_t *owi, const ds18b20_params_t *params);

int ds18b20_trigger(const ds18b20_t *dev);

int ds18b20_trigger_all(const ds18b20_t *dev);

int ds18b20_read(const ds18b20_t *dev, int16_t *temp);

int ds18b20_get_temp(const ds18b20_t *dev, int16_t *temp);


#ifdef __cplusplus
}
#endif

#endif /* DS18B20_H */
/** @} */
