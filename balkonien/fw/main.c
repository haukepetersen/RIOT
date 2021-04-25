/*
 * Copyright (C) 2021 Hauke Petersen <devel@haukeptersen.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     balkonien
 * @{
 *
 * @file
 * @brief       Balkonien Firmware
 *
 * @author      Hauke Petersen <devel@haukeptersen.de>
 *
 * @}
 */

#include <stdint.h>
#include <string.h>

#include "log.h"
#include "ds18b20.h"
#include "bh1750fvi.h"
#include "periph/adc.h"
#include "ztimer.h"

#define DEV_SOIL_0      (1 << 0)
#define DEV_SOIL_1      (1 << 1)
#define DEV_SOIL_2      (1 << 2)
#define DEV_SOIL_3      (1 << 3)
#define DEV_LIGHT_0     (1 << 7)
#define DEV_TEMP_0      (1 << 12)
#define DEV_TEMP_1      (1 << 13)
#define DEV_TEMP_2      (1 << 14)
#define DEV_TEMP_3      (1 << 15)

#define ADC_RES         ADC_RES_10BIT

static const bh1750fvi_params_t _bh1750_cfg = {
    .i2c = I2C_DEV(0),
    .addr = BH1750FVI_DEFAULT_ADDR,
};

static const onewire_params_t _onewire_cfg = {
    .pin = GPIO_PIN(1, 15),
    .pin_mode = GPIO_OUT,
};

static const ds18b20_params_t _ds18_cfg[] = {
    {
        .rom = { { 0x28, 0xFF, 0x18, 0x0C, 0x34, 0x16, 0x04, 0x7F } },
        .res = DS18B20_RES_12BIT,
    },
    {
        .rom = { { 0x28, 0xFF, 0x94, 0x09, 0x34, 0x16, 0x04, 0x14} },
        .res = DS18B20_RES_12BIT,
    },
    {
        .rom = { { 0x28, 0xFF, 0xF2, 0x11, 0x34, 0x16, 0x04, 0xBD} },
        .res = DS18B20_RES_12BIT,
    },
    {
        .rom = { {0x28, 0xFF, 0x07, 0x21, 0x34, 0x16, 0x04, 0x81} },
        .res = DS18B20_RES_12BIT,
    },
};

static const adc_t _soil[] = {
    ADC_LINE(1),
    ADC_LINE(2),
    ADC_LINE(4),
    ADC_LINE(5),
};

static onewire_t _onewire;
static bh1750fvi_t _light;
static ds18b20_t _temp[4];

static struct {
    uint32_t active;
} _state;

int main(void)
{
    int res;

    LOG_INFO("Balkonien V0.23\n");

    memset(&_state, 0, sizeof(_state));

    LOG_INFO("Iniitialize ADCs\n");
    for (unsigned i = 0; i < ARRAY_SIZE(_soil); i++) {
        res = adc_init(_soil[i]);
        if (res == 0) {
            _state.active |= (DEV_SOIL_0 << i);
        }
    }

    LOG_INFO("Iniitialize Light Sensor (BH1750FVI)\n");
    res = bh1750fvi_init(&_light, &_bh1750_cfg);
    if (res == 0) {
        _state.active |= DEV_LIGHT_0;
    }
    else {
        LOG_ERROR("err: Unable to initialize Light Sensor\n");
    }

    LOG_INFO("Initialize Temperature Sensors (DS18B20)\n");
    res = onewire_init(&_onewire, &_onewire_cfg);
    if (res == ONEWIRE_OK) {
        for (unsigned i = 0; i < ARRAY_SIZE(_temp); i++) {
            res = ds18b20_init(&_temp[i], &_onewire, &_ds18_cfg[i]);
            if (res == DS18B20_OK) {
                _state.active |= (DEV_TEMP_0 << i);
            }
            else {
                LOG_INFO("Unable to initialize Temp Sensor %u\n", i);
            }
        }
    }
    else {
        LOG_ERROR("err: Unable to initialize Onewire bus\n");
    }


    while (1) {
        int32_t val[4];
        for (unsigned i = 0; i < ARRAY_SIZE(_soil); i++) {
            val[i] = adc_sample(_soil[i], ADC_RES);
        }

        uint16_t illu = bh1750fvi_sample(&_light);
        printf("foo %4li %4li %4li %4li illu:%3u\n", val[0], val[1], val[2], val[3], (unsigned)illu);

        ds18b20_trigger_all(&_temp[0]);
        ztimer_sleep(ZTIMER_MSEC, 780);
        int16_t t[4];
        for (unsigned i = 0; i < ARRAY_SIZE(_temp); i++) {
            res = ds18b20_read(&_temp[i], &t[i]);
            if (res != DS18B20_OK) {
                printf("error reading temp %u\n", i);
            }
        }
        printf("temp: %i°C %i°C %i°C %i°C\n", (int)t[0], (int)t[1], (int)t[2], (int)t[3]);


    }



    return 0;
}
