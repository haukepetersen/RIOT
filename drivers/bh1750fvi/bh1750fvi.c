/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     drivers_bh1750fvi
 * @{
 *
 * @file
 * @brief       BH1750FVI ambient light sensor driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "ztimer.h"

#include "bh1750fvi.h"
#include "bh1750fvi_internal.h"

#define ENABLE_DEBUG        0
#include "debug.h"


int bh1750fvi_init(bh1750fvi_t *dev, const bh1750fvi_params_t *params)
{
    int res;

    /* initialize the device descriptor */
    dev->i2c = params->i2c;
    dev->addr = params->addr;

    /* initialize the I2C bus */
    i2c_acquire(dev->i2c);

    /* send a power down command to make sure we can speak to the device */
    res = i2c_write_byte(dev->i2c, dev->addr, OP_POWER_DOWN, 0);
    i2c_release(dev->i2c);
    if (res < 0) {
        return BH1750FVI_ERR_I2C;
    }
    return BH1750FVI_OK;
}


uint32_t bh1750fvi_get_conv_time_ms(const bh1750fvi_t *dev)
{
    (void)dev;

    return DELAY_HMODE_MS;
}

int bh1750fvi_trigger(const bh1750fvi_t *dev)
{
    int res;

    /* power on the device and send single H-mode measurement command */
    DEBUG("[bh1750fvi] sample: triggering a conversion\n");
    res = i2c_acquire(dev->i2c);
    if (res != 0) {
        return res;
    }
    res = i2c_write_byte(dev->i2c, dev->addr, OP_POWER_ON, 0);
    if (res != 0) {
        i2c_release(dev->i2c);
        return res;
    }
    res = i2c_write_byte(dev->i2c, dev->addr, OP_SINGLE_HRES1, 0);
    i2c_release(dev->i2c);
    return res;
}

int bh1750fvi_read(const bh1750fvi_t *dev, uint16_t *data)
{
    int res;
    uint32_t tmp;
    uint8_t raw[2];

    /* read the results */
    DEBUG("[bh1750fvi] sample: reading the results\n");
    res = i2c_acquire(dev->i2c);
    if (res != 0) {
        return res;
    }
    res = i2c_read_bytes(dev->i2c, dev->addr, raw, 2, 0);
    i2c_release(dev->i2c);

    /* on success we calculate the actual LUX value */
    if (res == 0) {
        tmp = ((uint32_t)raw[0] << 24) | ((uint32_t)raw[1] << 16);
        tmp /= RES_DIV;
        *data = (uint16_t)tmp;
    }

    return res;
}

uint16_t bh1750fvi_sample(const bh1750fvi_t *dev)
{
    int res;
    uint16_t data;

    res = bh1750fvi_trigger(dev);
    if (res != 0) {
        return 0;
    }

    /* wait for measurement to complete */
    ztimer_sleep(ZTIMER_MSEC, bh1750fvi_get_conv_time_ms(dev));

    res = bh1750fvi_read(dev, &data);
    if (res != 0) {
        return 0;
    }

    return data;
}
