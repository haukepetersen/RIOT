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
 * @brief       RIOT specific implementation of Gorm's event triggering timer
 *
 * @todo        Think about inlining functions, obvious candidates:
 *              - anchor()
 *              - cancel ()
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

/* TODO: remove */
#include "board.h"

#include "net/gorm/ll.h"
#include "net/gorm/arch/timer.h"

static void set_ctx(gorm_arch_timer_t *timer,
                    gorm_arch_timer_cb_t cb, void *arg)
{
    timer->base.callback = cb;
    timer->base.arg = arg;
}

/* also cancels the timer */
void gorm_arch_timer_anchor(gorm_arch_timer_t *timer, uint32_t compensation)
{
    timer->last = (xtimer_now_usec() - compensation);
}

/* Will change last value */
void gorm_arch_timer_set_from_last(gorm_arch_timer_t *timer, uint32_t timeout,
                                   gorm_arch_timer_cb_t cb, void *arg)
{
    set_ctx(timer, cb, arg);
    timer->last += timeout;
    xtimer_set(&timer->base, (timer->last - xtimer_now_usec()));
}

/* does not touch last value */
void gorm_arch_timer_set_from_now(gorm_arch_timer_t *timer, uint32_t timeout,
                                  gorm_arch_timer_cb_t cb, void *arg)
{
    set_ctx(timer, cb, arg);
    xtimer_set(&timer->base, timeout);
}

void gorm_arch_timer_cancel(gorm_arch_timer_t *timer)
{
    xtimer_remove(&timer->base);
}
