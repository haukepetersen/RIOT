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
#include "periph/adc.h"
#include "bh1750fvi.h"
#include "ztimer.h"

#define DEV_SOIL_0      (1 << 0)
#define DEV_SOIL_1      (1 << 1)
#define DEV_SOIL_2      (1 << 2)
#define DEV_SOIL_3      (1 << 3)
#define DEV_LIGHT_0     (1 << 7)

#define ADC_RES         ADC_RES_10BIT

static bh1750fvi_params_t _bh1750_cfg = {
    .i2c = I2C_DEV(0),
    .addr = BH1750FVI_DEFAULT_ADDR,
};

static adc_t _soil[] = {
    ADC_LINE(1), ADC_LINE(2), ADC_LINE(4), ADC_LINE(5),
};

static bh1750fvi_t _light;

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
    if (res != 0) {
        _state.active |= DEV_LIGHT_0;
    }
    else {
        LOG_ERROR("err: Unable to initialize Light Sensor\n");
    }


    while (1) {
        int32_t val[4];
        for (unsigned i = 0; i < ARRAY_SIZE(_soil); i++) {
            val[i] = adc_sample(_soil[i], ADC_RES);
        }

        uint16_t illu = bh1750fvi_sample(&_light);
        printf("foo %4li %4li %4li %4li illu:%3u\n", val[0], val[1], val[2], val[3], (unsigned)illu);

        ztimer_sleep(ZTIMER_MSEC, 100);
    }



    return 0;
}
