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

static void _off(void *arg)
{
    (void)arg;
    LED0_ON;
    for (int i = 0; i < 3000000; i++) {__asm("nop");}
    LED0_OFF;
    for (int i = 0; i < 3000000; i++) {__asm("nop");}
    LED0_ON;
    for (int i = 0; i < 3000000; i++) {__asm("nop");}
    LED0_OFF;
    for (int i = 0; i < 3000000; i++) {__asm("nop");}
    LED0_ON;
    for (int i = 0; i < 3000000; i++) {__asm("nop");}
    LED0_OFF;
    for (int i = 0; i < 3000000; i++) {__asm("nop");}
    LED0_ON;
    for (int i = 0; i < 3000000; i++) {__asm("nop");}
    LED0_OFF;

    NRF_POWER->SYSTEMOFF = 1;
}

int main(void)
{
    // puts("Hello World!");

    // printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    // printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    // NRF_CLOCK->TASKS_HFCLKSTOP = 1;

    printf("DCDC: 0x%08x\n", (int)NRF_POWER->DCDCEN);
    printf("HFXO: 0x%08x\n", (int)NRF_CLOCK->HFCLKSTAT);

    gpio_init_int(BTN0_PIN, BTN0_MODE, GPIO_FALLING, _off, NULL);

    // if (NRF_CLOCK->HFCLKSTAT) {
    //     LED0_OFF;
    // }
    // else {
    //     LED0_ON;
    // }

    return 0;
}
