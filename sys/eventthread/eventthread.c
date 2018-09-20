/*
 * Copyright (C) 2018 Freie Universität Berlin

 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_eventloop
 * @{
 *
 * @file
 * @brief       Global event loop implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "eventthread.h"

typedef struct {
    char stack[EVENTTHREAD_STACKSIZE];
    event_queue_t q;
} eventthread_ctx_t;

static eventthread_ctx_t _ctx[(EVENTTHREAD_COUNT - 1)];

void *_handler(void *arg)
{
    (void)arg;
    event_queue_init(&eventloop_queue);
    event_loop(&eventloop_queue);

    /* should be never reached */
    return NULL;
}

void eventloop_init(void)
{
    thread_create(_stack, sizeof(_stack), EVENTLOOP_PRIORITY, 0,
                  _handler, NULL, "eventloop");
}
