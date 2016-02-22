/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_nrf5x_common
 * @{
 *
 * @file
 * @brief       Implementation of the low-level I2C driver interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "cpu.h"
#include "mutex.h"
#include "periph/i2c.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

/**
 * @brief   Locks for making this driver thread safe
 */
static mutex_t locks[] = {
    MUTEX_INIT,
    MUTEX_INIT
};

static inline NRF_TWIM_Type *dev(i2c_t bus)
{
    return i2c_config[bus].dev;
}

static inline int check_error(i2c_t bus)
{
    dev(bus)->EVENTS_ERROR = 0;

    if (dev(bus)->ERRORSRC & TWIM_ERRORSRC_ANACK_Msk) {
        dev(bus)->ERRORSRC = TWIM_ERRORSRC_ANACK_Msk;
        DEBUG("[i2c] check_error: NACK on address byte\n");
        return I2C_ERR_ADDR;
    }
    if (dev(bus)->ERRORSRC & TWIM_ERRORSRC_DNACK_Msk) {
        dev(bus)->ERRORSRC = TWIM_ERRORSRC_DNACK_Msk;
        DEBUG("[i2c] check_error: NACK on data byte\n");
        return I2C_ERR_DATA;
    }

    DEBUG("[i2c] check_error: no error\n");
    return I2C_OK;
}

static int finish(i2c_t bus)
{
    DEBUG("[i2c] waiting for STOPPED or ERROR EVENT\n");
    check_error(bus);

    while ((!(dev(bus)->EVENTS_STOPPED)) && (!(dev(bus)->EVENTS_ERROR))
            && (!(dev(bus)->EVENTS_LASTTX))) {
        cpu_sleep();
        //DEBUG("wait\n");
    }

    if ((dev(bus)->EVENTS_LASTTX)) {
        DEBUG("[i2c] finish: go LASTTX event\n");
    }
    if ((dev(bus)->EVENTS_STOPPED)) {
        DEBUG("[i2c] finish: stop event occurred\n");
    }
    if (dev(bus)->EVENTS_ERROR) {
        DEBUG("[i2c] finish: error event occurred\n");
    }

    /* clear events */
    dev(bus)->EVENTS_STOPPED = 0;
    return check_error(bus);
}

int i2c_init(i2c_t bus)
{
    if (bus >= I2C_NUMOF) {
        return -1;
    }

    /* configure pins */
    mutex_lock(&locks[bus]);
    dev(bus)->PSEL.SDA = i2c_config[bus].sda_pin;
    dev(bus)->PSEL.SCL = i2c_config[bus].scl_pin;
    mutex_unlock(&locks[bus]);

    return 0;
}

int i2c_acquire(i2c_t bus, i2c_speed_t speed)
{
    if (speed & 0xff) {
        return -1;
    }

    DEBUG("[i2c] enabling bus %i\n", (int)bus);

    mutex_lock(&locks[bus]);
    dev(bus)->ENABLE = TWI_ENABLE_ENABLE_Enabled;
    dev(bus)->FREQUENCY = speed;

    return 0;
}

void i2c_release(i2c_t bus)
{
    dev(bus)->ENABLE = TWI_ENABLE_ENABLE_Disabled;
    mutex_unlock(&locks[bus]);

    DEBUG("[i2c] released bus %i\n", (int)bus);
}


int i2c_read(i2c_t bus, uint8_t addr, uint8_t *data, size_t len)
{
    DEBUG("[i2c] read: read %i byte from addr 0x%02x\n", (int)len, addr);

    dev(bus)->ADDRESS = addr;
    dev(bus)->RXD.PTR = (uint32_t)data;
    dev(bus)->RXD.MAXCNT = (uint8_t)len;
    dev(bus)->SHORTS =  TWIM_SHORTS_LASTRX_STOP_Msk;
    dev(bus)->TASKS_STARTRX = 1;

    return finish(bus);;
}

int i2c_read_reg(i2c_t bus, uint8_t addr, uint8_t reg, uint8_t *data, size_t len)
{
    DEBUG("[i2c] read %i byte from reg 0x%02x at addr 0x%02x\n",
           (int)len, reg, addr);

    dev(bus)->ADDRESS = addr;
    dev(bus)->TXD.PTR = (uint32_t)&reg;
    dev(bus)->TXD.MAXCNT = 1;
    dev(bus)->RXD.PTR = (uint32_t)data;
    dev(bus)->RXD.MAXCNT = (uint8_t)len;
    dev(bus)->SHORTS = (TWIM_SHORTS_LASTTX_STARTRX_Msk | TWIM_SHORTS_LASTRX_STOP_Msk);
    dev(bus)->TASKS_STARTTX = 1;

    return finish(bus);
}

int i2c_write_bytes(i2c_t bus, uint8_t addr, uint8_t *data, size_t len)
{
    DEBUG("[i2c] write %i byte to addr 0x%02x\n", (int)len, addr);

    dev(bus)->ADDRESS = addr;
    dev(bus)->TXD.PTR = (uint32_t)data;
    dev(bus)->TXD.MAXCNT = (uint8_t)len;
    dev(bus)->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;
    dev(bus)->TASKS_STARTTX = 1;

    return finish(bus);
}

int i2c_write_regs(i2c_t bus, uint8_t addr, uint8_t reg,
                   uint8_t *data, size_t len)
{
    DEBUG("[i2c] write %i byte to reg 0x%02x addr 0x%02x\n",
           (int)len, reg, addr);

    dev(bus)->ADDRESS = addr;
    dev(bus)->TXD.PTR = (uint32_t)&reg;
    dev(bus)->TXD.MAXCNT = 1;
    dev(bus)->SHORTS = TWIM_SHORTS_LASTTX_SUSPEND_Msk;
    dev(bus)->TASKS_STARTTX = 1;

    while (!(dev(bus)->EVENTS_LASTTX) && !(dev(bus)->EVENTS_ERROR)) {
        cpu_sleep();
    }
    dev(bus)->EVENTS_LASTTX = 0;

    if (check_error(bus) < 0) {
        return -1;
    }

    dev(bus)->TXD.PTR = (uint32_t)data;
    dev(bus)->TXD.MAXCNT = (uint8_t)len;
    dev(bus)->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;
    dev(bus)->TASKS_STARTTX = 1;

    return finish(bus);
}
