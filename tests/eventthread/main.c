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

#include "eventthread.h"
#include "thread.h"

#define RES         (0xa1b2c3b4)

typedef struct {
    event_t super;
    const char *text;
    unsigned order;
    uint8_t fingerprint;
} my_event_t;

static event_queue_t _main_q;

static uint32_t _res = 0;
static kernel_pid_t _pids[4];

static void _my_cb(event_t *arg);

static my_event_t _my1 = {
    .super.handler = _my_cb,
    .text = "Event 1 (lowprio)",
    .order = 1,
    .fingerprint = 0xb2,
};

static my_event_t _my2 = {
    .super.handler = _my_cb,
    .text = "Event 2 (lowprio)",
    .order = 2,
    .fingerprint = 0xc3,
};

static my_event_t _my3 = {
    .super.handler = _my_cb,
    .text = "Event 3 (drivers)",
    .order = 0,
    .fingerprint = 0xa1,
};

static my_event_t _my4 = {
    .super.handler = _my_cb,
    .text = "Event 4 (main)",
    .order = 3,
    .fingerprint = 0xb4
};

static void _my_cb(event_t *arg)
{
    my_event_t *my = (my_event_t *)arg;
    printf("This is '%s'\n", my->text);
    _res = ((_res << 8) | my->fingerprint);
    _pids[my->order] = thread_getpid();

    /* after 'Event 2' is executed, we want to re-enable the main thread */
    if (my->order == 2) {
        event_post(&_main_q, &_my4.super);
    }
}

int main(void)
{
    puts("Eventloop test application.\n");

    event_queue_init(&_main_q);

    eventthread_post(EVENTTHREAD_LOWPRIO, &_my1.super);
    eventthread_post(EVENTTHREAD_LOWPRIO, &_my2.super);
    eventthread_post(EVENTTHREAD_DRIVERS, &_my3.super);

    event_t *e = event_wait(&_main_q);
    e->handler(e);

    if ((_res == RES) &&
        (_pids[3] == thread_getpid()) &&
        (_pids[1] == _pids[2]) &&
        (_pids[0] != _pids[1]) &&
        (_pids[3] != _pids[1])) {
        puts("[SUCCESS]");
    }
    else {
        puts("[FAIL]");
    }

    return 0;
}
