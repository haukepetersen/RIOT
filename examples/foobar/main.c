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
#include "cpu.h"

#include "board.h"
#include "periph/gpio.h"
#include "ztimer/periodic.h"

static void _on_btn(void *arg)
{
    (void)arg;

    LED0_TOGGLE;
    LED1_TOGGLE;
    LED2_TOGGLE;
    LED3_TOGGLE;
}

static int _on_tim(void *arg)
{
    (void)arg;

    LED0_TOGGLE;
    LED1_TOGGLE;
    puts("tach tim");

    return 0;
}
    ztimer_periodic_t t;


int main(void)
{
    puts("moin");

    gpio_init_int(BTN3_PIN, GPIO_IN_PU, GPIO_FALLING, _on_btn, NULL);

    LED1_ON;
    for (int i = 0; i < 1000000; i++) {__asm("nop");}
    LED1_OFF;
    for (int i = 0; i < 1000000; i++) {__asm("nop");}
    LED1_ON;
    for (int i = 0; i < 1000000; i++) {__asm("nop");}
    LED1_OFF;
    for (int i = 0; i < 1000000; i++) {__asm("nop");}
    LED1_ON;
    for (int i = 0; i < 1000000; i++) {__asm("nop");}
    LED1_OFF;

    puts("achne");

    ztimer_periodic_init(ZTIMER_MSEC, &t, _on_tim, NULL, 500);
    puts("init ok");

    printf("addr cb: %p\n", (void *)*_on_tim);
    printf("addr cb t: %p\n", (void *)t.callback);
    ztimer_periodic_start(&t);
    puts("start ok");

    return 0;
}
