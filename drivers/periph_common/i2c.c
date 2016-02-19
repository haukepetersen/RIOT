/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers
 * @{
 *
 * @file
 * @brief       Common I2C functions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "periph/i2c.h"

#ifdef PERIPH_I2C_NEEDS_WRITE_BYTE
int i2c_write_byte(i2c_t bus, uint8_t addr, uint8_t data)
{
    return i2c_write_bytes(bus, addr, &data, 1);
}
#endif

#ifdef PERIPH_I2C_NEEDS_WRITE_REG
int i2c_write_reg(i2c_t bus, uint8_t addr, uint8_t reg, uint8_t data)
{
    return i2c_write_regs(bus, addr, reg, &data, 1);
}
#endif
