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
#include <string.h>

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

/* general register bitmasks */
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

/* control register bitmaps */
#define CTRL_EOSC           0x80
#define CTRL_BBSQW          0x40
#define CTRL_CONV           0x20
#define CTRL_RS2            0x10
#define CTRL_RS1            0x80
#define CTRL_RS             (CTRL_RS2 | CTRL_RS1)
#define CTRL_INTCN          0x40
#define CTRL_A2IE           0x20
#define CTRL_A1IE           0x10
#define CTRL_AIE            (CTRL_A2IE | CTRL_A1IE)

/* status register bitmaps */
#define STAT_OSF            0x80
#define STAT_EN32KHZ        0x08
#define STAT_BSY            0x04
#define STAT_A2F            0x02
#define STAT_A1F            0x01
#define STAT_AF             (STAT_A2F | STAT_A1F)

/* alarm configuration values */
#define ALARM_SEL           0x80
#define ALARM_MDAY          0x10
#define A1M1(type)          ((type & 0x01) << 7)
#define A1M2(type)          ((type & 0x02) << 6)
#define A1M3(type)          ((type & 0x04) << 5)
#define A1M4(type)          ((type & 0x08) << 4)
#define A2M2(type)          ((type & 0x02) << 6)
#define A2M3(type)          ((type & 0x04) << 5)
#define A2M4(type)          ((type & 0x08) << 4)


static int _read(const ds3231_t *dev, uint8_t reg, uint8_t *buf, size_t len,
                 int acquire, int release)
{
    int res;

    if (acquire && i2c_acquire(dev->bus)) {
        return -1;
    }
    res = i2c_read_regs(dev->bus, DS3231_I2C_ADDR, reg, buf, len, 0);
    if (res < 0) {
        i2c_release(dev->bus);
        return -1;
    }
    if (release) {
        i2c_release(dev->bus);
    }
    return 0;
}

static int _write(const ds3231_t *dev, uint8_t reg, uint8_t *buf, size_t len,
                  int acquire, int release)
{
    if (acquire && i2c_acquire(dev->bus)) {
        return -1;
    }
    if (i2c_write_regs(dev->bus, DS3231_I2C_ADDR, reg, buf, len, 0) < 0) {
        i2c_release(dev->bus);
        return -1;
    }
    if (release) {
        i2c_release(dev->bus);
    }
    return 0;
}

static int _setclr(const ds3231_t *dev, uint8_t reg,
                   uint8_t clr_mask, uint8_t set_mask, int aquire, int release)
{
    uint8_t old;
    uint8_t new;

    if (_read(dev, reg, &old, 1, aquire, 0) < 0) {
        return -1;
    }
    new = ((old &= ~clr_mask) | set_mask);
    if (_write(dev, reg, &new, 1, 0, release) < 0) {
        return -1;
    }
    return (int)old;
}

int ds3231_init(ds3231_t *dev, const ds3231_params_t *params)
{
    uint8_t ctrl;

    /* write device descriptor */
    memset(dev, 0, sizeof(ds3231_t));
    dev->bus = params->bus;

    /* try to read the control register, this also checks if we can talk to the
     * device */
    if (_read(dev, REG_CTRL, &ctrl, 1, 1, 0) < 0) {
        return -ENODEV;
    }
    /* disable interrupts, leave square wave generation and output pin
     * configuration untouched */
    ctrl &= ~(CTRL_A1IE | CTRL_A2IE);
    /* if configured, start the oscillator. Also disable interrupts */
    if (params->opt & DS3231_POWERON_ON_INIT) {
        ctrl &= ~CTRL_EOSC;
    }

    if (_write(dev, REG_CTRL, &ctrl, 1, 0, 1) < 0) {
        return -EIO;
    }

    return 0;
}

int ds3231_get_time(const ds3231_t *dev, struct tm *time)
{
    uint8_t raw[DATE_REG_NUMOF];

    /* read date registers */
    if (_read(dev, REG_SEC, raw, DATE_REG_NUMOF, 1, 1) < 0) {
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
    raw[REG_DAY] = bcd_from_byte(time->tm_wday + 1);
    raw[REG_DATE] = bcd_from_byte(time->tm_mday);
    raw[REG_MONTH] = bcd_from_byte(time->tm_mon + 1);
    raw[REG_YEAR] = bcd_from_byte(time->tm_year % 100);
    if (time->tm_year > 100) {
        raw[REG_MONTH] |= MASK_CENTURY;
    }

    /* write time to device */
    if (_write(dev, REG_SEC, raw, DATE_REG_NUMOF, 1, 1) < 0) {
        return -EIO;
    }

    return 0;
}

int ds3231_set_alarm(const ds3231_t *dev, ds3231_alarm_type_t type,
                     const struct tm *time)
{
    size_t len;
    uint8_t areg;
    uint8_t en_bit;
    uint8_t raw[A1_REG_NUMOF];

    /* clear any alarm that might be currently enabled */
    if (_setclr(dev, REG_CTRL, CTRL_AIE, 0, 1, 0) < 0) {
        return -EIO;
    }
    if (_setclr(dev, REG_STATUS, STAT_AF, 0, 0, 0) < 0) {
        return -EIO;
    }

    if (type < ALARM_SEL) {
        /* setting alarm 1 */
        raw[0] = bcd_from_byte(time->tm_sec) | A1M1(type);
        raw[1] = bcd_from_byte(time->tm_min) | A1M2(type);
        raw[2] = bcd_from_byte(time->tm_hour) | A1M3(type);
        if (type & ALARM_MDAY) {
            raw[3] = bcd_from_byte(time->tm_mday + 1) | A1M4(type);
        }
        else {
            raw[3] = bcd_from_byte(time->tm_wday + 1) | MASK_DY_DT | A1M4(type);
        }
        len = A1_REG_NUMOF;
        areg = REG_A1_SEC;
        en_bit = CTRL_A1IE;
    }
    else {
        /* setting alarm 2 */
        raw[0] = bcd_from_byte(time->tm_min) | A2M2(type);
        raw[1] = bcd_from_byte(time->tm_hour) | A2M3(type);
        if (type & ALARM_MDAY) {
            raw[2] = bcd_from_byte(time->tm_mday + 1) | A2M4(type);
        }
        else {
            raw[2] = bcd_from_byte(time->tm_wday + 1) | MASK_DY_DT | A2M4(type);
        }
        len = A2_REG_NUMOF;
        areg = REG_A2_MIN;
        en_bit = CTRL_A2IE;
    }

    if (_write(dev, areg, raw, len, 0, 0) < 0) {
        return -EIO;
    }
    if (_setclr(dev, REG_CTRL, 0, en_bit, 0, 1) < 0) {
        return -EIO;
    }

    return 0;
}

int ds3231_check_and_clear_alarm(const ds3231_t *dev)
{
    int res;
    uint8_t ctrl;

    if (_read(dev, REG_CTRL, &ctrl, 1, 1, 0) < 0) {
        return -EIO;
    }
    if (!(ctrl & (CTRL_A1IE | CTRL_A2IE))) {
        /* no alarm configured */
        return -ENOENT;
    }

    res = _setclr(dev, REG_STATUS, (STAT_A1F | STAT_A2F), 0, 1, 1);
    if (res < 0) {
        return -EIO;
    }

    return (res & (STAT_A1F | STAT_A2F)) ? 1 : 0;
}

int ds3231_remove_alarm(const ds3231_t *dev)
{
    return (_setclr(dev, REG_CTRL, CTRL_AIE, 0, 1, 1) >= 0) ? 0 : -EIO;
}

int ds3231_squarewave_enable(const ds3231_t *dev, ds3231_sqw_t type)
{
    uint8_t clr = (CTRL_RS | CTRL_INTCN);
    uint8_t set = ((type & CTRL_RS) | CTRL_BBSQW);
    return (_setclr(dev, REG_CTRL, clr, set, 1, 1) >= 0) ? 0 : -EIO;
}

int ds3231_squarewave_disable(const ds3231_t *dev)
{
    return (_setclr(dev, REG_CTRL, CTRL_BBSQW, CTRL_INTCN, 1, 1) >= 0) ? 0
                                                                       : -EIO;
}

int ds3231_poweroff(const ds3231_t *dev)
{
    return (_setclr(dev, REG_CTRL, 0, CTRL_EOSC, 1, 1) >= 0) ? 0 : -EIO;
}

int ds3231_poweron(const ds3231_t *dev)
{
    return (_setclr(dev, REG_CTRL, CTRL_EOSC, 0, 1, 1) >= 0) ? 0 : -EIO;
}
