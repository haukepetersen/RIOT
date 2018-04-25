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
 * @brief       Gorm's event abstraction
 *
 * @note        All timer callbacks are supposed to be run in interrupt context
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_ARCH_EVENT_H
#define NET_GORM_ARCH_EVENT_H

/**
 * @todo    this include is RIOT specific, so move...
 */
#include "event.h"

/**
 * @brief   RIOT specific event definition
 *
 * @todo    move to RIOT specific header somehow
 */
typedef struct {
    event_t super;      /**< base timer */
    void *ctx;          /**< Gorm context tied to the event */
    uint16_t state;     /**< event flags marking pending event type(s) */
} gorm_arch_evt_t;

/**
 * @brief   Event callback signature
 *
 * @param[in] event     event that was triggered
 */
typedef void (*gorm_arch_evt_cb_t)(gorm_arch_evt_t *event);

/**
 * @brief   Initialize the given event
 *
 * @param[out] event    initialize this event
 * @param[in]  ctx      set context for the event
 * @param[in]  cb       callback called when the event is triggered
 */
void gorm_arch_evt_init(gorm_arch_evt_t *event,
                        void *ctx, gorm_arch_evt_cb_t cb);

/**
 * @brief   Post an event
 *
 * @param[in] event     event to trigger
 * @param[in] type      flag for marking the type of event
 */
void gorm_arch_evt_post(gorm_arch_evt_t *event, uint16_t type);

/**
 * @brief   Run an event loop that waits for new events
 *
 * This function will block the calling thread indefinitely waiting for events.
 */
void gorm_arch_evt_loop(void);

#endif /* NET_GORM_ARCH_EVENT_H */
/** @} */
