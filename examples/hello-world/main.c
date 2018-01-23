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

#include "xtimer.h"
#include "sx150x.h"
#include "sx150x_params.h"

static sx150x_t sx;

#define PIN   (15U)

int main(void)
{
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);


    sx150x_init(&sx, &sx150x_params[0]);

    sx150x_gpio_init(&sx, PIN, GPIO_OUT);
    // sx150x_gpio_init(&sx, 15, GPIO_OUT);


    while (1) {
        // sx150x_gpio_set(&sx, PIN);
        // printf("pin %i on", (int)PIN);
        xtimer_usleep(500 * 1000);

        // sx150x_gpio_clear(&sx, PIN);
        // printf("pin %i off", (int)PIN);
        xtimer_usleep(500 * 1000);

        // sx150x_gpio_toggle(&sx, 15);
    }

    return 0;
}
