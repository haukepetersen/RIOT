/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm_arch
 * @{
 *
 * @file
 * @brief       RIOT specific implementation of Gorm's event abstraction
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "irq.h"

#include "net/gorm/arch/evt.h"

/* allocate the event queue */
static event_queue_t _queue;

void gorm_arch_evt_init(gorm_arch_evt_t *event,
                          void *ctx, gorm_arch_evt_cb_t cb)
{
    event->super.handler = (event_handler_t)cb;
    event->ctx = ctx;
    event->state = 0;
}

void gorm_arch_evt_post(gorm_arch_evt_t *event, uint16_t type)
{
    /* only allow events to be posted once the event loop is running */
    if (_queue.waiter == NULL) {
        return;
    }

    unsigned is = irq_disable();
    event->state |= type;
    event_post(&_queue, &event->super);
    irq_restore(is);
}

void gorm_arch_evt_loop(void)
{
    event_queue_init(&_queue);
    event_loop(&_queue);
}
