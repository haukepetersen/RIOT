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


#include "thread.h"
#include "mm.h"

int main(void)
{
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    MM_INIT;

    MM1H;
    MM2T;
    MM2T;
    MM1L;

    MM1H;
    MM1L;

    MM1H;
    asm("nop");
    MM1L;

    MM1H;
    asm("nop");
    asm("nop");
    MM1L;

    MM1H;
    asm("nop");
    asm("nop");
    asm("nop");
    MM1L;

    MM1H;
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    MM1L;

    MM1H;
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    MM1L;


    MM1H;
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    MM1L;

    // gpio_init(GPIO_PIN(PA,6), GPIO_DIR_OUT, GPIO_NOPULL);
    // while (1) {
    //     gpio_toggle(GPIO_PIN(PA,6));
    //     MM2T;
    //     for (int i = 0; i < 500000; i++) {
    //         asm("nop");
    //     }
    // }

    return 0;
}
