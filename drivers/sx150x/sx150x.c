/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_sx150x
 * @{
 *
 * @file
 * @brief       SX150X driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#include <string.h>

#include "assert.h"
#include "sx150x.h"

#ifdef MODULE_SX1509
#include "sx1509_regs.h"
#else
#include "sx15078_regs.h"
#endif

#define ENABLE_DEBUG (1)
#include "debug.h"

#define I2C_CLK             I2C_SPEED_FAST

#define CHECK(expr)         if (expr != SX150X_OK) { goto err; }

/* define device module specific configuration */
#ifdef MODULE_SX1509
#define MASK(pin)           (1 << (pin & 0x7))
#else
#define MASK(pin)           (1 << pin)
#endif

static int _reg_read(const sx150x_t *dev, uint8_t reg, uint8_t *val)
{
    if (i2c_read_reg(dev->bus, dev->addr, reg, val) != 1) {
        return SX150X_BUSERR;
    }
    return SX150X_OK;
}

static int _reg_write(const sx150x_t *dev, uint8_t reg, uint8_t val)
{
    // DEBUG("write reg 0x%02x, val 0x%02x\n", (int)reg, (int)val);
    if (i2c_write_reg(dev->bus, dev->addr, reg, val) != 1) {
        return SX150X_BUSERR;
    }
    return SX150X_OK;
}

// static int _reg_write_u16(const sx150x_t *dev, uint8_t reg, uint16_t val)
// {
//     if (i2c_write_regs(dev->bus, dev->addr, reg, &val, 2) != 2) {
//         return SX150X_BUSERR;
//     }
//     return SX150X_OK;
// }

static int _reg_set(const sx150x_t *dev, uint8_t reg, uint8_t mask)
{
    uint8_t tmp;
    if (_reg_read(dev, reg, &tmp) != SX150X_OK) {
        return SX150X_BUSERR;
    }
    DEBUG("--SET [0x%02x], before 0x%02x\n", (int)reg, (int)tmp);
    tmp |= mask;
    DEBUG("--SET [0x%02x], after  0x%02x\n", (int)reg, (int)tmp);
    return _reg_write(dev, reg, tmp);
}

static int _reg_clr(const sx150x_t *dev, uint8_t reg, uint8_t mask)
{
    uint8_t tmp;
    if (_reg_read(dev, reg, &tmp) != SX150X_OK) {
        return SX150X_BUSERR;
    }
    DEBUG("--CLR [0x%02x], before 0x%02x\n", (int)reg, (int)tmp);
    tmp &= ~(mask);
    DEBUG("--CLR [0x%02x], before 0x%02x\n", (int)reg, (int)tmp);
    return _reg_write(dev, reg, tmp);
}

// static int reg_tgl(const sx150x_t *dev, uint8_t reg, uint8_t mask)
// {
//     uint8_t tmp;
//     if (_reg_read(dev, reg, &tmp) != SX150X_OK) {
//         return SX150X_BUSERR;
//     }
//     tmp ^= mask;
//     return _reg_write(dev, reg, tmp);
// }

int sx150x_init(sx150x_t *dev, const sx150x_params_t *params)
{
    assert(dev && params);

    int res = SX150X_OK;
    uint8_t tmp;

    memcpy(dev, params, sizeof(sx150x_params_t));

    DEBUG("[sx150x] init: initializing driver now\n");

    /* initialize I2C bus */
    if (i2c_init_master(dev->bus, I2C_CLK) != 0) {
        DEBUG("[sx150x] init: error - unable to initialize I2C bus\n");
        res = SX150X_NOBUS;
        goto exit;
    }

    DEBUG("[sx150x] init master ok\n");

    /* try to get access to the bus */
    if (i2c_acquire(dev->bus) != 0) {
        DEBUG("[sx150x] init: unable to acquire bus\n");
        res = SX150X_BUSERR;
        goto exit;
    }

    DEBUG("[sx150x] acquire ok\n");

    /* do SW reset and read input disable register for testing the connection */
    res = _reg_write(dev, REG_RESET, RESET_SEQ_1);
    if (res != SX150X_OK) {
        goto exit;
    }
    res = _reg_write(dev, REG_RESET, RESET_SEQ_2);
    if (res != SX150X_OK) {
        goto exit;
    }

    res = _reg_read(dev, REG_INPUT_DISABLE(0), &tmp);
    if ((res != SX150X_OK) || (tmp != 0)) {
        DEBUG("[sx150x] init: error - unable to read from device\n");
        res = SX150X_NODEV;
        goto exit;
    }

    DEBUG("[sx150x] init: initialization successful\n");

exit:
    i2c_release(dev->bus);
    return res;
}

// #include "xtimer.h"
// static void sleep(void)
// {
//     xtimer_usleep(250 * 1000);
// }
/* REMOVE */
// static void dump(const sx150x_t *dev)
// {
//     for (uint8_t reg = 0x0a; reg <= 0x11; reg++) {
//         uint8_t tmp;
//         _reg_read(dev, reg, &tmp);
//         printf("Reg 0x%02x := 0x%02x\n", (int)reg, (int)tmp);
//     }
// }

static void val(sx150x_t *dev, uint8_t reg, const char *out)
{
    uint8_t tmp;
    _reg_read(dev, reg, &tmp);
    DEBUG("%s (0x%02x) is 0x%02x\n", out, (int)reg, (int)tmp);
}

static void stat(sx150x_t *dev)
{
    val(dev, REG_OPEN_DRAIN(0), "OPEN_DRAIN");
    val(dev, REG_OPEN_DRAIN(8), "OPEN_DRAIN");
    val(dev, REG_DIR(0), "DIR");
    val(dev, REG_DIR(8), "DIR");
    val(dev, REG_DATA(0), "DATA");
    val(dev, REG_DATA(8), "DATA");
}

int sx150x_gpio_init(sx150x_t *dev, unsigned pin, gpio_mode_t mode)
{
    assert(dev && (pin < SX150X_PIN_NUMOF));
    DEBUG("init pin %i\n", (int)pin);

    (void)mode;
    i2c_acquire(dev->bus);
    // CHECK(_reg_set(dev, REG_INPUT_DISABLE(pin), MASK(pin)));
    CHECK(_reg_clr(dev, REG_DIR(pin), MASK(pin)));
    // CHECK(_reg_set(dev, REG_OPEN_DRAIN(pin), MASK(pin)));
    stat(dev);
    i2c_release(dev->bus);
    return SX150X_OK;

err:
    i2c_release(dev->bus);
    return SX150X_BUSERR;
}

int sx150x_gpio_set(sx150x_t *dev, unsigned pin)
{
    assert(dev && (pin < SX150X_PIN_NUMOF));
    DEBUG("[sx150x] gpio_set: pin %i (mask 0x%02x)\n", (int)pin, (int)MASK(pin));

    i2c_acquire(dev->bus);
    CHECK(_reg_set(dev, REG_DATA(pin), MASK(pin)));
    stat(dev);
    i2c_release(dev->bus);
    return SX150X_OK;

err:
    i2c_release(dev->bus);
    return SX150X_BUSERR;
}

int sx150x_gpio_clear(sx150x_t *dev, unsigned pin)
{
    assert(dev && (pin < SX150X_PIN_NUMOF));
    DEBUG("[sx150x] gpio_clear: pin %i (mask 0x%02x)\n", (int)pin, (int)MASK(pin));

    i2c_acquire(dev->bus);
    CHECK(_reg_clr(dev, REG_DATA(pin), MASK(pin)));
    stat(dev);
    i2c_release(dev->bus);
    return SX150X_OK;

err:
    i2c_release(dev->bus);
    return SX150X_BUSERR;
}

// int sx150x_gpio_toggle(sx150x_t *dev, unsigned pin)
// {
//     assert(dev && (pin < SX150X_PIN_NUMOF));

//     i2c_acquire(dev->bus);
//     CHECK(reg_tgl(dev, REG_DATA(pin), MASK(pin)));

//     i2c_release(dev->bus);
//     return SX150X_OK;
// err:
//     i2c_release(dev->bus);
//     return SX150X_BUSERR;
// }


// int sx150x_write(sx150x_t *dev, unsigned pin, int value)
// {
//     if (value) {
//         return sx150x_set(dev, pin);
//     }
//     else {
//         return sx150x_clear(dev, pin);
//     }
// }
