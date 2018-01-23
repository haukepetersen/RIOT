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


    puts("hello: init\n");
    sx150x_init(&sx, &sx150x_params[0]);

    printf("\nhello: gpio init, pin %i\n", (int)PIN);
    sx150x_gpio_init(&sx, PIN, GPIO_OUT);
    puts("hello: set pin 15");
    sx150x_gpio_set(&sx, PIN);

    printf("\nhello: gpio_init, pin 13\n");
    sx150x_gpio_init(&sx, 13, GPIO_OUT);
    puts("hello: set pin 13");
    sx150x_gpio_set(&sx, 13);
    puts("hello: clear pin 15");
    sx150x_gpio_clear(&sx, 15);


    printf("\nhello: gpio_init, pin 7\n");
    sx150x_gpio_init(&sx, 7, GPIO_OUT);
    puts("hello: set pin 7");
    sx150x_gpio_set(&sx, 7);
    // puts("hello: clear pin 7");
    // sx150x_gpio_clear(&sx, 7);


    // xtimer_usleep(500 * 1000);

    // puts("hello: clear pin\n");
    // sx150x_gpio_clear(&sx, PIN);

    puts("hello: done here.\n");
    return 0;
}
