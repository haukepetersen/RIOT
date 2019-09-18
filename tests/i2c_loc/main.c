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

    for (unsigned sda = 3; sda < 32; sda++) {
        // if (sda == 6 || sda == 8) {
        //     continue;
        // }
        for (unsigned scl = 3; scl < 32; scl++) {
            if (scl == sda) {
                continue;
            }
            // if (scl == 6 || scl == 8) {
            //     continue;
            // }
            printf("RUN scl: %u, sda: %u\n", scl, sda);
            i2c_init_custom(0, scl, sda);



            // for (unsigned p = 3; p < 32; p++) {
            //     if (p != scl && p != sda) {
            //         gpio_init(GPIO_PIN(0, p), GPIO_OUT);
            //         gpio_set(GPIO_PIN(0, p));
            //     }
            // }

            for (unsigned addr = 0; addr < 256; addr++) {
                uint8_t tmp;
                i2c_acquire(0);
                int res = i2c_read_reg(0, (uint16_t)addr, 0, &tmp, 0);
                i2c_release(0);

                if (res == 0) {
                    if (tmp != 0x48) {
                        printf("GOT a valid response: 0x%02x!\n", (int)tmp);
                    }
                }
                else if (res == -ENXIO) {
                    // printf(" -> 0x%02x: no dev (ENXIO)\n", addr);
                }
                else if (res == -ETIMEDOUT) {
                    if (addr == 0x00) {
                        printf(" -> ETIMEDOUT\n");
                    }
                }
                else if (res == -EIO) {
                    printf(" -> 0x%02x: no ACK (EIO)\n", addr);
                }
                else {
                    printf(" -> 0x%02x: some err (%i)\n", addr, res);
                }
                xtimer_usleep(2000);
            }
        }
    }





    return 0;
}
