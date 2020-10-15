/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     drivers_ds3231
 * @{
 *
 * @file
 * @brief       DS3231 RTC driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <time.h>
#include <stdio.h>

#include "bcd.h"

#include "ds3231.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

/* some metadata about the devices registers */
#define DATE_REG_NUMOF      (7U)
#define A1_REG_NUMOF        (4U)
#define A2_REG_NUMOF        (3U)

/* register addresses */
#define REG_SEC             0x00
#define REG_MIN             0x01
#define REG_HOUR            0x02
#define REG_DAY             0x03
#define REG_DATE            0x04
#define REG_MONTH           0x05
#define REG_YEAR            0x06
#define REG_A1_SEC          0x07
#define REG_A1_MIN          0x08
#define REG_A1_HOUR         0x09
#define REG_A1_DAYDATE      0x0a
#define REG_A2_MIN          0x0b
#define REG_A2_HOUR         0x0c
#define REG_A2_DAYDATE      0x0d
#define REG_CTRL            0x0e
#define REG_STATUS          0x0f
#define REG_AGING_OFFSET    0x10
#define REG_TEMP_MSB        0x11
#define REG_TEMP_LSB        0x12

/* register bitmasks */
#define MASK_SEC10          0x70
#define MASK_SEC            0x0f
#define MASK_MIN10          0x70
#define MASK_MIN            0x0f
#define MASK_H10            0x10
#define MASK_AMPM_H20       0x20
#define MASK_H12_H24        0x40
#define MASK_H20H10         (MASK_H10 | MASK_AMPM_H20)
#define MASK_HOUR           0x0f
#define MASK_DAY            0x07
#define MASK_DATE10         0x30
#define MASK_DATE           0x0f
#define MASK_MONTH10        0x10
#define MASK_MONTH          0x0f
#define MASK_CENTURY        0x80
#define MASK_DY_DT          0x60

#define CTRL_EOSC           0x80
#define CTRL_BBSQW          0x40
#define CTRL_CONV           0x20
#define CTRL_RS2            0x10
#define CTRL_RS1            0x80
#define CTRL_INTCN          0x40
#define CTRL_A2IE           0x20
#define CTRL_A1IE           0x10

#define STAT_OSF            0x80
#define STAT_EN32KHZ        0x08
#define STAT_BSY            0x04
#define STAT_A2F            0x02
#define STAT_A1F            0x01



int ds3231_init(ds3231_t *dev, const ds3231_params_t *params)
{
    int res;
    uint8_t ctrl;

    /* write device descriptor */
    memset(dev, 0, sizeof(ds3231_t));
    dev->bus = params->bus;
    dev->pin_int = params->pin_int;

    /* start the oscillator, if not already running */
    res = i2c_acquire(dev->bus);
    if (res != 0) {
        return -EIO;
    }
    res = i2c_read_reg(dev->bus, DS3231_I2C_ADDR, REG_CTRL, &ctrl, 0);
    if (res != 0) {
        i2c_release(dev->bus);
        return -ENODEV;
    }

    /* enable clock and disable interrupts, as at this point
    /* start the oscillator if configured */
    if (params->opt & DS3231_POWERON_ON_INIT && (ctrl & CTRL_EOSC)) {
        ctrl &= ~CTRL_EOSC;
        res = i2c_write_reg(dev->bus, DS3231_I2C_ADDR, REG_CTRL, ctrl, 0);
        if (res != 0) {
            res = -EIO;
        }
    }

    i2c_release(res);

    return res;
}

int ds3231_init_int(ds3231_t *dev, ds3231_cb_t cb, void *arg)
{

}

int ds3231_get_time(const ds3231_t *dev, struct tm *time)
{
    int res;
    uint8_t raw[DATE_REG_NUMOF];

    /* read date registers */
    res = i2c_acquire(dev->bus);
    if (res != 0) {
        return -EIO;
    }
    res = i2c_read_regs(dev->bus, DS3231_I2C_ADDR, REG_SEC,
                        raw, DATE_REG_NUMOF, 0);
    /* we do need to release the bus also in case of a transfer error above... */
    i2c_release(dev->bus);
    if (res != 0) {
        return -EIO;
    }

    /* convert data to struct tm */
    time->tm_sec = bcd_to_byte(raw[REG_SEC]);
    time->tm_min = bcd_to_byte(raw[REG_MIN]);
    if (raw[REG_HOUR] & MASK_H12_H24) {
        time->tm_hour = bcd_to_byte(raw[REG_HOUR] & (MASK_HOUR | MASK_H10));
        if (raw[REG_HOUR] & MASK_AMPM_H20) {
            time->tm_hour += 12;
        }
    }
    else {
        time->tm_hour = bcd_to_byte(raw[REG_HOUR] & (MASK_HOUR | MASK_H20H10));
    }
    time->tm_mday = bcd_to_byte(raw[REG_DATE]);
    time->tm_mon = bcd_to_byte(raw[REG_MONTH] & ~MASK_CENTURY) - 1;
    time->tm_year = bcd_to_byte(raw[REG_YEAR]);
    if (raw[REG_MONTH] & MASK_CENTURY) {
        time->tm_year += 100;
    }
    time->tm_wday = bcd_to_byte(raw[REG_DAY]) - 1;

    return 0;
}

int ds3231_set_time(const ds3231_t *dev, const struct tm *time)
{
    int res;
    uint8_t raw[DATE_REG_NUMOF];

    /* some validity checks */
    if (time->tm_year > 200) {
        return -ERANGE;
    }

    /* convert struct tm to raw BDC data */
    raw[REG_SEC] = bcd_from_byte(time->tm_sec);
    raw[REG_MIN] = bcd_from_byte(time->tm_min);
    /* note: we always set the hours in 24-hour format */
    raw[REG_HOUR] = bcd_from_byte(time->tm_hour);
    raw[REG_DAY] = bcd_from_byte(time->tm_wday) + 1;
    raw[REG_DATE] = bcd_from_byte(time->tm_mday);
    raw[REG_MONTH] = bcd_from_byte(time->tm_mon) + 1;
    raw[REG_YEAR] = bcd_from_byte(time->tm_year % 100);
    if (time->tm_year > 100) {
        raw[REG_MONTH] |= MASK_CENTURY;
    }

    /* write time to device */
    res = i2c_acquire(dev->bus);
    if (res != 0) {
        return -EIO;
    }
    res = i2c_write_regs(dev->bus, DS3231_I2C_ADDR, REG_SEC,
                         raw, DATE_REG_NUMOF, 0);
    /* release the bus in any case ... */
    i2c_release(dev->bus);
    if (res != 0) {
        return -EIO;
    }

    return 0;
}

int ds3231_set_alarm(const ds3231_t *dev, ds3231_alarm_type_t type,
                     const struct tm *time);

int ds3231_check_alarm(const ds3231_t *dev)
{
    int res;
    int ret = 0;
    uint8_t tmp[2];

    res = i2c_acquire(dev->bus);
    if (res != 0) {
        return -EIO;
    }
    res = i2c_read_regs(dev->bus, DS3231_I2C_ADDR, REG_CTRL, tmp, 2, 0);
    i2c_release(dev->bus);
    if (res != 0) {
        return -EIO;
    }

    if (!(tmp[0] & (CTRL_A1IE | CTRL_A2IE))) {
        /* no alarm set */
        return -ENOENT;
    }
    if (((tmp[0] & CTRL_A1IE) && (tmp[1] & STAT_A1F)) ||
        ((tmp[0] & CTRL_A2IE) && (tmp[1] & STAT_A2F))) {
        /* alarm has been triggered */

        ret = 1;

    }


    return ret;
}


int ds3231_check_alarm(const ds3231_t *dev, unsigned chan)
{
    int res;
    uint8_t tmp;

    res = i2c_acquire(dev->bus);
    if (res != 0) {
        return -EIO;
    }
    res i2c_read_reg(dev->bus, DS3231_I2C_ADDR, REG_STATUS, &tmp, 0);
    i2c_release(dev->bus);
    if (res != 0) {
        return -EIO;
    }

    uint8_t flag =
}

int ds3231_clear_alarm(const ds3231_t *dev)
{
    int res;
    uint8_t tmp[2];

    res = i2c_acquire(dev->bus);
    if (res != 0) {
        return -EIO;
    }
    res = i2c_read_regs(dev->bus, DS3231_I2C_ADDR, REG_CTRL, tmp, 2, 0);
    if (res != 0) {
        i2c_release(dev->bus);
        return -EIO;
    }
    tmp[0] &= ~(CTRL_A1IE | CTRL_A2IE);
    tmp[1] &= ~(STAT_A1F | STAT_A2F);
    res = i2c_write_regs(dev->bus, DS3231_I2C_ADDR, REG_CTRL, tmp, 2, 0);
    if (res != 0) {
        res = -EIO;
    }
    i2c_release(dev->bus);

    return res;
}

int ds3231_alarm_disable(const ds3231_t *dev)
{

}

int ds3231_poweroff(const ds3231_t *dev)
{
    int res;
    uint8_t tmp;

    res = i2c_acquire(dev->bus);
    if (res != 0) {
        return -EIO;
    }
    res = i2c_read_reg(dev->bus, DS3231_I2C_ADDR, REG_CTRL, &tmp, 0);
    if (res != 0) {
        i2c_release(dev->bus);
        return -EIO;
    }
    if (tmp & CTRL_OSF) {
        tmp &= ~CTRL_OSF;
        res = i2c_write_reg(dev->bus, DS3231_I2C_ADDR, REG_CTRL, tmp, 0);
        if (res != 0) {
            res = -EIO;
        }
    }
    i2c_release(dev->bus);
    return res;
}

int ds3231_poweron(const ds3231_t *dev)
{
    int res;
    uint8_t tmp;

    res = i2c_acquire(dev->bus);
    if (res != 0) {
        return -EIO;
    }
    res = i2c_read_reg(dev->bus, DS3231_I2C_ADDR, REG_CTRL, &tmp, 0);
    if (res != 0) {
        i2c_release(dev->bus);
        return -EIO;
    }
    if (!(tmp & CTRL_OSF)) {
        tmp |= CTRL_OSF;
        res = i2c_write_reg(dev->bus, DS3231_I2C_ADDR, REG_CTRL, tmp, 0);
        if (res != 0) {
            res = -EIO;
        }
    }
    i2c_release(dev->bus);
    return res;
}
