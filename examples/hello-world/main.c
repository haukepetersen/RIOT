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

int main(void)
{
    // puts("Hello World!");

    // printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    // printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    // NRF_CLOCK->TASKS_HFCLKSTOP = 1;

    for (unsigned i = 0; i < 9; i++) {
        printf("RAM[%i].POWER -> 0x%08x\n", i, (int)NRF_POWER->RAM[i].POWER);
    }

    // if (NRF_CLOCK->HFCLKSTAT) {
    //     LED0_OFF;
    // }
    // else {
    //     LED0_ON;
    // }

    return 0;
}
