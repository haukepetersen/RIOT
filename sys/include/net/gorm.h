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
 * @brief       Gorm is a light-weight BLE stack for RIOT
 *
 * @warning THIS FEATURE IS EXPERIMENTAL
 *
 * # About
 *
 * TODO
 *
 * # General Design
 *
 * # Design Objectives
 * - focused on SoCs with included, memory mapped BLE radio
 * - leight-weight
 * - taking shortcuts from the standard here and there -> skipping HCI
 *
 *
 * # Implementation Status
 *
 * Supported roles (as of now):
 * - broadcaster
 * - peripheral
 *
 *
 *
 * # Todos
 *
 * This is a list of things that need to be done in the future, and serves as a
 * rather loose collection of things that should be part of Gorm. This list is
 * in no means complete, but is also not just a diff between implemented
 * features and the BT standard...
 *
 * ## General
 * @todo        enable SCANNER role
 * @todo        enable CENTRAL (GATT client) role
 * @todo        implement L2CAP reliable channel -> IPSP
 * @todo        implement IPv6 over BLE (IPSP)
 *
 * ## Security Manager
 * @todo        Implement
 *
 * ## Link-layer
 * @todo        support channel selection algorithm #2
 * @todo        support extended packet sized
 *
 * ## Implementation specific
 * @todo        cleanup the (sub-)module structure to allow compiling Gorm for
 *              different roles while keeping code-size down
 *
 * TODO sort/fix...
 * @todo        Unify concept of static vs dynamic configuaration values, e.g.
 *             - advertisement intervals (conn vs nonconn)
 * @todo        Address handling: generation of public addresses etc
 * @todo        Use netdev correctly (do proper switching to thread context)
 * @todo        Implement and use `event_post_first()` and use in link layer for
 *              received packets
 * @todo        check for consistent naming of static functions (_x vs x)
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
    GORM_OK           =  0,     /**< everything went well */
    GORM_ERR_CTX_BUSY = -1,     /**< the given context is not free to use */
    GORM_ERR_NOBUF    = -2,     /**< no buffer space available */
    GORM_ERR_DEV      = -3,     /**< BLE radio device error */
};

/**
 * @brief   Context data holding the full context for each connection
 */
typedef struct {
    gorm_ll_ctx_t ll;       /**< link-layer context, needs to be first field */
    gorm_arch_evt_t event;  /**< event used for synchronizing ISRs and thread */
    /* add more context fields for higher level protocols on demand (e.g. SM) */
} gorm_ctx_t;

/**
 * @brief   Initialize Gorm
 */
void gorm_init(netdev_t *radio);

/**
 * @brief   Run Gorm's event loop, servicing the upper (host) layers
 */
void gorm_run(void);

/**
 * @brief   Notify Gorm's thread about link layer events
 *
 * @param[in] con       Connection context that spawned the event
 * @param[in] type      Event type
 */
void gorm_notify(gorm_ctx_t *con, uint16_t type);

/**
 * @brief   Start advertising the default GAP data using a free context
 *
 * @return  GORM_OK if advertising was started or advertising is already in
 *          progress
 * @return  GORM_ERR_CTX_BUSY if no free context is available
 */
int gorm_adv_start(void);

/**
 * @brief   Stop advertising the default GAP data
 */
void gorm_adv_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* GORM_H */
/** @} */
