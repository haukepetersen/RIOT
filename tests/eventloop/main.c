/*
 * Copyright (C) 2018 Freie Universität Berlin
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
 * @brief       Test the global event loop
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "eventloop.h"
#include "thread.h"

typedef struct {
    event_t super;
    const char *text;
    kernel_pid_t pid_test;
} my_event_t;

static void _my_cb(event_t *arg);

static my_event_t _my1 = {
    .super.handler = _my_cb,
    .text = "Event 1",
    .pid_test = 0
};

static my_event_t _my2 = {
    .super.handler = _my_cb,
    .text = "Event 2",
    .pid_test = 0
};

static void _my_cb(event_t *arg)
{
    my_event_t *my = (my_event_t *)arg;
    printf("This is '%s'\n", my->text);
    my->pid_test = thread_getpid();
}

int main(void)
{
    puts("Eventloop test application.\n");

    kernel_pid_t main_pid = thread_getpid();

    eventloop_post(&_my1.super);
    eventloop_post(&_my2.super);

    if ((_my1.test_pid != main_pid) && (_my1.test_pid > 0) &&
        (_my2.test_pid != main_pid) && (_my2.test_pid > 0)) {
        puts("[SUCCESS]");
    }
    else {
        puts("[FAIL]");
    }

    return 0;
}
