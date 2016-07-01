/*
 * Copyright (C) 2016 Freie Universität Berlin
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
    DS18B20_NO_DEV      = -1,
    DS18B20_ERR_RES     = -2
};

typedef enum {
    DS18B20_RES_9BIT    = 0x1f,     /**< conversion time ~93.75ms */
    DS18B20_RES_10BIT   = 0x3f,     /**< conversion time ~187.5ms */
    DS18B20_RES_11BIT   = 0x5f,     /**< conversion time ~375ms */
    DS18B20_RES_12BIT   = 0x7f      /**< conversion time ~750ms */
} ds18b20_res_t;

typedef struct {
    onewire_t bus;
    onewire_rom_t rom;
    ds18b20_res_t res;
} ds18b20_params_t;

typedef struct {
    ds18b20_params_t p;
    uint32_t t_conv;
} ds18b20_t;

void ds18b20_setup(ds18b20_t *dev, const ds18b20_params_t *params);

int ds18b20_init(ds18b20_t *dev);

void ds18b20_trigger(ds18b20_t *dev);

int16_t ds18b20_read(ds18b20_t *dev);

int16_t ds18b20_get_temp(ds18b20_t *dev);


#ifdef __cplusplus
}
#endif

#endif /* DS18B20_H */
/** @} */
