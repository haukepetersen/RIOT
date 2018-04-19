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

#include "event.h"

typedef struct {
    event_t super;
    void *ctx;
    uint16_t state;
} gorm_arch_evt_t;

typedef void (*gorm_arch_evt_cb_t)(gorm_arch_evt_t *event);

void gorm_arch_evt_init(gorm_arch_evt_t *event,
                          void *ctx, gorm_arch_evt_cb_t cb);

void gorm_arch_evt_post(gorm_arch_evt_t *event, uint16_t type);

void gorm_arch_evt_loop(void);

#endif /* NET_GORM_ARCH_EVENT_H */
/** @} */
