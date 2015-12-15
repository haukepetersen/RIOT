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
#include "msg.h"
#include "xtimer.h"
#include "dbgpin.h"

char stack[THREAD_STACKSIZE_DEFAULT];
char stack2[THREAD_STACKSIZE_DEFAULT];
char stack3[THREAD_STACKSIZE_DEFAULT];

void sig(void)
{
    MM1L;
    MM2L;
    xtimer_usleep(1);
    // MM2H;
    // xtimer_usleep(1);
    // MM2L;
    // xtimer_usleep(1);
}

void *second_thread(void *arg)
{
    (void) arg;
    MM1L;
    MM2H;
    return NULL;
}

void *msg_thread(void *arg)
{
    (void)arg;
    msg_t m;
    puts("msg_thread");

    while (1) {
        msg_receive(&m);
        MM1L;
        MM2H;
    }

    return NULL;
}

void *sleep_thread(void *arg)
{
    (void)arg;
    puts("sleep_thread");

    while (1) {
        thread_sleep();
        MM1L;
        MM2H;
    }

    return NULL;
}

int main(void)
{
    msg_t m;
    puts("Hello World!");


    kernel_pid_t t2 = thread_create(
            stack2, sizeof(stack2),
            THREAD_PRIORITY_MAIN - 2, CREATE_STACKTEST,
            msg_thread, NULL, "nr2");

    kernel_pid_t t3 = thread_create(
            stack3, sizeof(stack3),
            THREAD_PRIORITY_MAIN - 3, CREATE_STACKTEST,
            sleep_thread, NULL, "nr2");

    (void) thread_create(
            stack, sizeof(stack),
            THREAD_PRIORITY_MAIN - 1, CREATE_WOUT_YIELD | CREATE_STACKTEST,
            second_thread, NULL, "nr1");

    puts("starting...");
    sig();

    MM1H;
    thread_yield();
    sig();

    MM1H;
    msg_send(&m, t2);
    sig();

    MM1H;
    thread_wakeup(t3);
    sig();

    puts("done.");
    return 0;
}
