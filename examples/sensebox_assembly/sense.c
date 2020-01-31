/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Initialize and read sensors
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdint.h>

#include "assert.h"
#include "xtimer.h"
#include "hdc1000.h"
#include "tsl4531x.h"
#include "ds18.h"
#include "periph/gpio.h"
#include "periph/adc.h"

#include "app.h"
#include "sense_params.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

static const hdc1000_params_t _hdc1000_params = {
    .i2c            = I2C_DEV(0),
    .addr           = HDC1000_I2C_ADDRESS,
    .res            = HDC1000_14BIT,
    .renew_interval = 1000000ul,
};

static const tsl4531x_params_t _tsl4531x_params = {
    .i2c_dev          = I2C_DEV(0),
    .i2c_addr         = TSL45315_ADDR,
    .integration_time = TSL4531X_INTEGRATE_400MS,
    .low_power_mode   = false,
    .part_number      = TSL45315_PARTNO,
};

static const ds18_params_t _ds18_params = {
    .pin      = GPIO_PIN(PB, 9),
    .out_mode = GPIO_OD_PU,
};

static const adc_t _hygro_params[] = {
    ADC_LINE(0),
    ADC_LINE(1),
    ADC_LINE(2),
    ADC_LINE(3),
};

static const appdata_type_t _sense_typecfg[] = {
    SENSOR_DATA_T_TEMP,
    SENSOR_DATA_T_HUM,
    SENSOR_DATA_T_UINT16,
    SENSOR_DATA_T_TEMP,
    SENSOR_DATA_T_UINT16,
    SENSOR_DATA_T_UINT16,
    SENSOR_DATA_T_UINT16,
    SENSOR_DATA_T_UINT16,
};

static hdc1000_t _hdc1000;
static tsl4531x_t _tsl4531x;
static ds18_t _ds18;

static void _pwr_on(uint32_t delay)
{
    gpio_set(SENSE_PWR_PIN);
    xtimer_sleep(delay);
    DEBUG("[sense] power on\n");
}

static void _pwr_off(void)
{
    gpio_clear(SENSE_PWR_PIN);
    DEBUG("[sense] power off\n");
}


int sense_init(void)
{
    int res;

    /* initialize power pin that controls the hygrometers as well as the DS18
     * sensor */
    res = gpio_init(SENSE_PWR_PIN, GPIO_OUT);
    if (res != 0) {
        DEBUG("[sense] error initializing power pin (%i)\n", res);
        return -1;
    }
    _pwr_on(SENSE_PWR_STARTDELAY);

    res = hdc1000_init(&_hdc1000, &_hdc1000_params);
    if (res != HDC1000_OK) {
        DEBUG("[sense] error initializing HDC1000 sensor (%i)\n", res);
        return -1;
    }

    res = tsl4531x_init(&_tsl4531x, &_tsl4531x_params);
    if (res != 0) {
        DEBUG("[sense] error initializing TSL4531x sensor(%i)\n", res);
        return -1;
    }

    res = ds18_init(&_ds18, &_ds18_params);
    if (res != 0) {
        DEBUG("[sense] error initializing DS18 sensor(%i)\n", res);
        return -1;
    }

    for (unsigned i = 0; i < HYGRO_NUMOF; i++) {
        res = adc_init(_hygro_params[i]);
        if (res != 0) {
            DEBUG("[sense] error initializing hygrometer #%u\n", i);
            return -1;
        }
    }

    _pwr_off();
    return 0;
}

int sense_hwtest(void)
{
    DEBUG("[sense] hardware selftest, cycling power %u times\n",
          SENSE_PWR_TESTCYCLES);
    for (unsigned i = 0; i < SENSE_PWR_TESTCYCLES; i++) {
        _pwr_on(SENSE_PWR_TESTDELAY);
        _pwr_off();
        xtimer_sleep(SENSE_PWR_TESTDELAY);
    }

    return 0;
}

int sense_readall(appdata_t *data)
{
    int res;
    assert(data);
    int16_t temp;
    int16_t hum;
    int32_t hyg;


    _pwr_on(SENSE_PWR_STARTDELAY);

    res = hdc1000_read_cached(&_hdc1000, &temp, &hum);
    if (res == HDC1000_OK) {
        data[0].raw = (uint16_t)temp;
        data[1].raw = (uint16_t)hum;
    }
    else {
        DEBUG("[sense] error reading HDC1000 sensor\n");
        data[0].raw = 0;
        data[1].raw = 0;
    }

    DEBUG("FOOBAR\n");

    /* read light value */
    data[2].raw = (int16_t)tsl4531x_simple_read(&_tsl4531x);

    /* read soil temperature */

    res = ds18_get_temperature(&_ds18, &temp);
    if (res == 0) {
        data[3].raw = (uint16_t)temp;
    }
    if (res != 0) {
        DEBUG("[sense] error reading DS18 sensor\n");
        data[3].raw = 0;
    }

    for (unsigned i = 0; i < HYGRO_NUMOF; i++) {
        hyg = adc_sample(_hygro_params[i], HYGRO_RES);
        if (hyg >= 0) {
            data[i + 4].raw = (uint16_t)hyg;
        }
        else {
            DEBUG("[sense] error reading hygrometer #%u\n", i);
            data[i + 4].raw = 0;
        }
    }

    /* set data type values */
    for (unsigned i = 0; i < SENSE_NUMOF; i++) {
        data[i].type = _sense_typecfg[i];
        DEBUG("[sense] raw data - sensor #%u: %4i (type: %i)\n", i,
              data[i].raw, data[i].type);
    }

    _pwr_off();
    return 0;
}
