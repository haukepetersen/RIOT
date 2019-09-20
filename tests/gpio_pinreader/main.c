/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <periph/gpio.h>
#include <xtimer.h>

uint8_t pins[] = { 4,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                  20, 21, 22, 23, 24, 27, 28, 29, 30, 31 };

void readall(gpio_mode_t mode, const char *desc)
{
    for (unsigned i = 0; i < sizeof(pins); i++) {
        gpio_init(GPIO_PIN(0, pins[i]), mode);
    }
    (void)desc;
    // printf("%s\n", desc);
    for (unsigned i = 0; i < sizeof(pins); i++) {
        int val = (gpio_read(GPIO_PIN(0, pins[i]))) ? 1 : 0;
        printf(" %i  ", val);
    }
    puts("");
}

int main(void)
{
    puts("Pin reader!");

    for (unsigned i = 0; i < sizeof(pins); i++) {
        printf("P%02i ", (int)pins[i]);
    }
    puts("");
    xtimer_sleep(2);
    while (1) {

        readall(GPIO_IN, "GPIO_IN");
        xtimer_usleep(100 * 1000);
        // readall(GPIO_IN_PU, "GPIO_IN_PU");
        // readall(GPIO_IN_PD, "GPIO_IN_PD");
    }

    return 0;
}
