/*
 * Copyright (C) 2014-2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     driver_isl29020
 * @{
 *
 * @file
 * @brief       Device driver implementation for the ISL29020 light sensor
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include "isl29020.h"
#include "isl29020-internal.h"
#include "periph/i2c.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

int isl29020_init(isl29020_t *dev, const isl29020_params_t *params)
{
    int res;
    char tmp;

    /* initialize device descriptor */
    dev->i2c     = params->i2c;
    dev->address = params->addr;
    dev->lux_fac = (float)((1 << (10 + (2 * params->range))) - 1) / 0xffff;

    /* acquire exclusive access to the bus */
    i2c_acquire(dev->i2c);
    /* initialize the I2C bus */
    i2c_init_master(dev->i2c, I2C_SPEED_NORMAL);

    /* configure and enable the sensor */
    tmp = (ISL29020_CMD_EN | ISL29020_CMD_MODE | ISL29020_RES_INT_16 |
           params->range | (params->mode << 5));
    res = i2c_write_reg(dev->i2c, dev->address, ISL29020_REG_CMD, tmp);
    /* release the bus for other threads */
    i2c_release(dev->i2c);
    if (res < 1) {
        return -1;
    }
    return 0;
}

int isl29020_read(isl29020_t *dev)
{
    char low, high;
    uint16_t res;
    int ret;

    i2c_acquire(dev->i2c);
    /* read lighting value */
    ret = i2c_read_reg(dev->i2c, dev->address, ISL29020_REG_LDATA, &low);
    ret += i2c_read_reg(dev->i2c, dev->address, ISL29020_REG_HDATA, &high);
    i2c_release(dev->i2c);
    if (ret < 2) {
        return -1;
    }
    res = (high << 8) | low;
    DEBUG("ISL29020: Raw value: %i - high: %i, low: %i\n", res, high, low);
    /* calculate and return the actual lux value */
    return (int)(dev->lux_fac * res);
}

int isl29020_enable(isl29020_t *dev)
{
    int res;
    char tmp;

    i2c_acquire(dev->i2c);
    res = i2c_read_reg(dev->i2c, dev->address, ISL29020_REG_CMD, &tmp);
    if (res < 1) {
        i2c_release(dev->i2c);
        return -1;
    }
    tmp |= ISL29020_CMD_EN;
    res = i2c_write_reg(dev->i2c, dev->address, ISL29020_REG_CMD, tmp);
    if (res < 1) {
        i2c_release(dev->i2c);
        return -1;
    }
    i2c_release(dev->i2c);
    return 0;
}

int isl29020_disable(isl29020_t *dev)
{
    int res;
    char tmp;

    i2c_acquire(dev->i2c);
    res = i2c_read_reg(dev->i2c, dev->address, ISL29020_REG_CMD, &tmp);
    if (res < 1) {
        i2c_release(dev->i2c);
        return -1;
    }
    tmp &= ~(ISL29020_CMD_EN);
    res = i2c_write_reg(dev->i2c, dev->address, ISL29020_REG_CMD, tmp);
    if (res < 1) {
        i2c_release(dev->i2c);
        return -1;
    }
    i2c_release(dev->i2c);
    return 0;
}
