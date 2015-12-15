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
 * @author      Ludwig Ortmann <ludwig.ortmann@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "thread.h"
#include "periph/timer.h"
#include "dbgpin.h"

void delay(void)
{
    for (int i = 0; i < 100; i++) {
        asm("nop");
    }
}

void evt(int c)
{
    (void)c;
    MM1H;
    delay();
    MM1L;
}

int main(void)
{
    puts("starting...");

    timer_init(0, 1, evt);

    timer_set(0, 0, 1000);

    return 0;
}
