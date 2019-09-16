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

static gpio_t _pin[29];
static xtimer_t _tim[29];

static void _on_tim(void *arg)
{
    unsigned p = (unsigned)arg;

    gpio_toggle(_pin[p]);
    xtimer_set(&_tim[p], ((p + 3) * 1000));
}

int main(void)
{
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    puts("We now have some fun with pin toggling\n");

    for (unsigned p = 0; p < 29; p++) {
        _pin[p] = GPIO_PIN(0, (p + 3));
        gpio_init(_pin[p], GPIO_OUT);
        gpio_clear(_pin[p]);

        _tim[p].callback = _on_tim;
        _tim[p].arg = (void *)p;

        xtimer_set(&_tim[p], ((p + 3) * 1000));
    }



    return 0;
}
