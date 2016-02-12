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
#include <stdint.h>

#include "board.h"
#include "thread.h"
#include "xtimer.h"

#define PRINT_DELAY         (1500 * 1000U)
#define TOGGLE_DELAY        (200 * 1000U)

char stack[THREAD_STACKSIZE_MAIN];

void *second_thread(void *arg)
{
    (void) arg;
    uint32_t now = xtimer_now();

    while (1) {
        LED_RED_TOGGLE;
        xtimer_usleep_until(&now, TOGGLE_DELAY);
    }

    return NULL;
}

int main(void)
{
    uint32_t now = xtimer_now();

    thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1, 0,
                  second_thread, NULL, "toggle");

    while (1) {
        puts("Hello World!");
        xtimer_usleep_until(&now, PRINT_DELAY);
    }

    return 0;
}
