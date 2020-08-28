/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     drivers_vl53l0x
 * @{
 *
 * @file
 * @brief       VL53L0X time-of-flight distance sensor driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "xtimer.h"

#include "vl53l0x.h"
#include "vl53l0x_internal.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"


static uint16_t _read_reg(const vl53l0x_t *dev, uint8_t reg, uint16_t *val)
{
    return i2c_read_regs(dev->bus, dev->addr, reg, val, 2, 0);
}

int vl53l0x_init(vl53l0x_t *dev, const vl53l0x_params_t *params)
{
    int res;
    uint16_t val;
    dev->bus = params->bus;

    res = _read_reg(dev, REG_IDENTIFICATION_MODEL_ID, &val);
    if ((res != 0) || (val != IDENTIFICATION_MODEL_ID)) {
        DEBUG("[vl53l0x] unable to read IDENTIFICATION_MODEL_ID\n");
        return VL53L0X_NODEV;
    }

    return VL53L0X_OK;
}

int vl53l0x_write_slave_addr(vl53l0x_t *dev, uint8_t new_addr)
{
    (void)dev;
    (void)new_addr;
    return VL53L0X_OK;
}
