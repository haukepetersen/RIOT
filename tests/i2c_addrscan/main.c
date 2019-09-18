/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Custom I2C finder
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <errno.h>

#include "xtimer.h"
#include "periph/i2c.h"
#include "periph/gpio.h"


extern void i2c_init_custom(i2c_t dev, unsigned scl, unsigned sda);

int main(void)
{
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    puts("We now have some fun with pin toggling\n");

    // check pins
    unsigned scl = NRF_TWIM1->PSEL.SCL;
    unsigned sda = NRF_TWIM1->PSEL.SDA;

    printf("I2C pins: SCL %u, SDA %u\n", scl, sda);


    for (unsigned addr = 0; addr <= 0xff; addr++) {
        unsigned reg = 0;
        uint8_t data;

        printf("ADDR: 0x%02x ###################\n", addr);

        i2c_acquire(I2C_DEV(0));
        int res = i2c_read_reg(I2C_DEV(0), (uint16_t)addr, (uint8_t)reg, &data, 0);
        i2c_release(I2C_DEV(0));

        if (res == 0) {
            printf("reg 0x%02x: got 0x%02x\n", reg, (int)data);
        }


        for (reg = 1; reg <= 0xff; reg++) {
            i2c_acquire(I2C_DEV(0));
            res = i2c_read_reg(I2C_DEV(0), (uint16_t)addr, reg, &data, 0);
            i2c_release(I2C_DEV(0));

            if (res == 0) {
                printf("reg 0x%02x: got 0x%02x\n", (int)reg, (int)data);
            }
        }

        xtimer_usleep(250000);
    }


    return 0;
}
