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

#ifndef NET_GORM_ARCH_TIMER_H
#define NET_GORM_ARCH_TIMER_H

#include "xtimer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   OS specific timer data
 *
 * @todo    This should be defined by the OS somewhere
 */
typedef struct {
    xtimer_t base;      /**< the actual timer struct used */
    uint32_t last;      /**< last used timeout, for offset compensation */
} gorm_arch_timer_t;

/**
 * @brief   Gorm specific timer callback signature
 *
 * @param[in] arg       user define argument
 */
typedef void (*gorm_arch_timer_cb_t)(void *arg);

/**
 * @brief   Set the time base to the current time minus the compensation value
 *
 * @param[out] timer        set base time for this timer
 * @param[in]  compensation offset compensation value
 */
void gorm_arch_timer_anchor(gorm_arch_timer_t *timer, uint32_t compensation);

/**
 * @brief   Set a timer to trigger @p timeout us after the last time it triggered
 *
 * This function will also update the last timeout value of the timer.
 *
 * @param[in,out] timer     timer to set
 * @param[in]     timeout   timeout in us
 * @param[in]     cb        callback to call on timeout
 * @param[in]     arg       argument passed to @p cb
 */
void gorm_arch_timer_set_from_last(gorm_arch_timer_t *timer, uint32_t timeout,
                                   gorm_arch_timer_cb_t cb, void *arg);

/**
 * @brief   Set a timer to trigger @p timeout us from the current time
 *
 * This function will not update the last timeout value save by the timer.
 *
 * @param[in,out] timer     timer to set
 * @param[in]     timeout   timeout in us
 * @param[in]     cb        callback to call on timeout
 * @param[in]     arg       argument passed to @p cb
 */
void gorm_arch_timer_set_from_now(gorm_arch_timer_t *timer, uint32_t timeout,
                                  gorm_arch_timer_cb_t cb, void *arg);

/**
 * @brief   Cancel the given timer
 *
 * @param[in] timer         timer to cancel
 */
void gorm_arch_timer_cancel(gorm_arch_timer_t *timer);

#ifdef __cplusplus
}
#endif

#endif /* NET_GORM_ARCH_TIMER_H */
/** @} */
