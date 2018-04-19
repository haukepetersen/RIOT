/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm Gorm, the daddy of Bluetooth
 * @ingroup     net
 * @brief       Gorm, the BLE stack for RIOT
 *
 * TODO more high-level doc
 *
 * @todo        Unify concept of static vs dynamic configuaration values, e.g.
 *              - advertisement intervals (conn vs nonconn)
 * @todo        Address handling: generation of public addresses etc
 * @todo        Use netdev correctly (do proper switching to thread context)
 * @todo        Implement and use `event_post_first()` and use in link layer for
 *              received packets
 * @todo        check for consistent naming of static functions (_x vs x)
 *
 * # Implementation Status
 *
 * ## Link-layer
 * Next feature to add
 * - support for channel selection algorithm #2
 *
 * @{
 * @file
 * @brief       High level interface for telling Gorm how to behave
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_H
#define GORM_H

#include <stdint.h>

#include "net/gorm/ll.h"
#include "net/gorm/config.h"
#include "net/gorm/arch/evt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Event types send from the controller to the host
 * @{
 */
#define GORM_EVT_DATA               (0x0001)
#define GORM_EVT_CONNECTED          (0x0002)
#define GORM_EVT_CON_ABORT          (0x0004)
#define GORM_EVT_CON_TIMEOUT        (0x0008)
#define GORM_EVT_CON_CLOSED         (0x0010)
/** @} */

/**
 * @brief   Generic error codes used by Gorm
 */
enum {
    GORM_OK     = 0,
    GORM_ERR_TIMINGS = -1,
    GORM_ERR_CTX_BUSY = -2,     /**< the given context is not free to use */
    GORM_ERR_NOBUF    = -3,     /**< no buffer space available */
    GORM_ERR_DEV      = -4,     /**< BLE radio device error */
};

typedef struct {
    gorm_ll_ctx_t ll;       /**< link-layer context, needs to be first field */
    gorm_arch_evt_t event;  /**< event used for synchronizing ISRs and thread */
    /* TODO: add other contexts (L2CAP, ...) */
    /* allocate memory for sending events to the host thread */
    // gorm_gap_ctx_t gap;
    // gorm_gatt_ctx_t gatt;
    // gorm_l2cap_ctx_t l2cap;
} gorm_ctx_t;

/**
 * @brief   Initialize Gorm's host implementation
 */
void gorm_init(netdev_t *radio);

/**
 * @brief   Run Gorm's host layers
 */
void gorm_run(void);


void gorm_notify(gorm_ctx_t *con, uint16_t type);


void gorm_adv_start(void);

void gorm_adv_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* GORM_H */
/** @} */
