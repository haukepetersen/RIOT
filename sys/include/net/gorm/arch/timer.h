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
 * @brief       Gorm's timer abstraction
 *
 * @note        All timer callbacks are supposed to be run in interrupt context
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_ARCH_TIMER_H
#define GORM_ARCH_TIMER_H

#include "xtimer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: let architecture specific implementation define this somehow */
typedef struct {
    xtimer_t base;
    uint32_t last;
} gorm_arch_timer_t;

typedef void (*gorm_arch_timer_cb_t)(void *);

/* does set last value to now() */
void gorm_arch_timer_anchor(gorm_arch_timer_t *timer, uint32_t compensation);

/* will change last value to last + timeout */
void gorm_arch_timer_set_from_last(gorm_arch_timer_t *timer, uint32_t timeout,
                                   gorm_arch_timer_cb_t cb, void *arg);

/* does not touch last value */
void gorm_arch_timer_set_from_now(gorm_arch_timer_t *timer, uint32_t timeout,
                                  gorm_arch_timer_cb_t cb, void *arg);

/* cancel the timer */
void gorm_arch_timer_cancel(gorm_arch_timer_t *timer);

#ifdef __cplusplus
}
#endif

#endif /* GORM_ARCH_TIMER_H */
/** @} */
