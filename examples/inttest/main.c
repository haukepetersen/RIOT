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
#include "periph/gpio.h"
#include "dbgpin.h"

char stack[THREAD_STACKSIZE_DEFAULT];
kernel_pid_t t1;

void delay(void)
{
    for (int i = 0; i < 100; i++) {
        asm("nop");
    }
}

void evt(void *arg)
{
    (void)arg;
    msg_t m;
    MM1H;
    msg_send_int(&m, t1);
}

void *rcv_thread(void *arg)
{
    (void)arg;
    msg_t m;

    while (1) {
        msg_receive(&m);
        MM1L;
    }
}

int main(void)
{
    puts("starting...");

    t1 = thread_create(stack, sizeof(stack), (THREAD_PRIORITY_MAIN - 1), 0, rcv_thread, NULL, "t1");

    MM1L;
    gpio_init_int(PB, GPIO_NOPULL, GPIO_RISING, evt, NULL);

    return 0;
}
