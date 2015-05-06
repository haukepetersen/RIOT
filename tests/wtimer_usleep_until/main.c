/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
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
 * @brief       timer test application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <stdio.h>
#include "hwtimer.h"
#include "wtimer.h"
#include "periph_conf.h"

#define N 1000

uint32_t res[N];

int main(void)
{
    puts("wtimer_usleep_until test application.\n");

    uint32_t interval = 1000;

    for (int i = 0; i < N; i++) {
        printf("Testing interval %u...\n", (unsigned)interval);
        uint32_t last_wakeup = wtimer_now();
        uint32_t before = last_wakeup;
        wtimer_usleep_until(&last_wakeup, (unsigned)interval);
        uint32_t diff = (last_wakeup-before)-interval;
        res[i] = diff;
        interval -= 1;
    }

    for (int i = 0; i < N; i++) {
        printf("%4d diff=%i\n", i, (int)res[i]);
    }

    printf("\nTest complete.\n");
    return 0;
}
