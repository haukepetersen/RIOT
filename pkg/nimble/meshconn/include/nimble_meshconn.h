/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    pkg_nimble_meshconn Meshconn
 * @ingroup     pkg_nimble
 * @brief       IP over BLE connection manager
 *
 * @experimental
 *
 * # WARNING
 * This module is highly experimental! Expect bugs, instabilities and sudden API
 * changes
 *
 *
 * # About
 * Meshconn is a generic BLE connection manager for IP over BLE environments
 * that can be used independently of the used IP routing protocol
 *
 * # Dependencies
 * Meshconn depends on the 6lo ND ABRO option.
 *
 *
 * # Concept
 * Roles: router nodes, leaf nodes
 * Behavior:
 *  routers -> advertising their presence incl. meta data
 *  leafs   -> scanning for routers, connecting to the ones with best fitness
 *              function results
 *
 * On connection loss: instantly try to reconnect for time X
 *
 *
 * # Usage
 * TODO
 *
 *
 * # Configuration
 * TODO
 *
 * # Design
 * Payloads:
 * ADV_IND/XXX: AD, flags + service data fields
 *   - service data: | 32-bit netID | 8-bit obj. func. type | 32-bit obj. func. |
 * -> 3 + 4 + 9 := 16 byte payload
 *
 * # Events (and what to do):
 * States:
 * - IDLE
 * - NOTCONN
 * - RECONN:
 *   - connectable, directed advertisements (ADV_DIRECT_IND / XXX (ext adv))
 *   - high duty cycle connectable directed advertising
 * - GROUNDED
 * - FLOATING



State:
- idle: not connected to anyone
- grounded: connection with upstream route
- floating: no upstream route connection

- reconn: instantly reconnecting lost connection


- advertising: only if upstream route exits
- scanning: only when searching for upstream route
- connecting: after selection of (additional) upstream route

 *
 * BLE events:
 * -
 * Timing events:
 *
 * @{
 *
 * @file
 * @brief       TODO
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NIMBLE_MESHCONN_H
#define NIMBLE_MESHCONN_H

#include <errno.h>
#include <stdint.h>

#include "list.h"
#include "nimble_netif.h"

#ifdef __cplusplus
extern "C" {
#endif


#define NIMBLE_MESHCONN_NOTCONN_SCAN_ITVL_MS        100
#define NIMBLE_MESHCONN_NOTCONN_SCAN_WIN_MS         10
#define NIMBLE_MESHCONN_NOTCONN_TIMEOUT             5000  // DEBUG -> increase in practice?

#define NIMBLE_MESHCONN_RECONN_SCAN_ITVL_MS         20
#define NIMBLE_MESHCONN_RECONN_SCAN_WIN_MS          20
#define NIMBLE_MESHCONN_RECONN_ADV_ITVL_MS          20
#define NIMBLE_MESHCONN_RECONN_TIMEOUT              80

#define NIMBLE_MESHCONN_GROUNDED_SCAN_ITVL_MS       250
#define NIMBLE_MESHCONN_GROUNDED_SCAN_WIN_MS        10
#define NIMBLE_MESHCONN_GROUNDED_ADV_ITVL_MS        100

#define NIMBLE_MESHCONN_CONN_ITVL_MIN_MS            60
#define NIMBLE_MESHCONN_CONN_ITVL_MAX_MS            80
#define NIMBLE_MESHCONN_CONN_SLAVE_LATENCY          0
#define NIMBLE_MESHCONN_CONN_SUPERVISION_TO_MS      2500
#define NIMBLE_MEHSCONN_CONN_TIMEOUT_MS             2500

#define NIMBLE_MESHCONN_LINKS_UP_MIN                1
#define NIMBLE_MESHCONN_LINKS_UP_MAX                1

typedef enum {
    // NIMBLE_MESHCONN_GROUNDED,
    NIMBLE_MESHCONN_RECONNECT,
    NIMBLE_MESHCONN_CONN_UPLINK,
    NIMBLE_MESHCONN_CONN_DOWNLINK,
    NIMBLE_MESHCONN_CLOSE_UPLINK,
    NIMBLE_MESHCONN_CLOSE_DOWNLINK,
    NIMBLE_MESHCONN_OF_UPDATE,
} nimble_meshconn_event_t;

typedef void(*nimble_meshconn_eventcb_t)(nimble_meshconn_event_t event,
                                         int handle,
                                         const uint8_t *addr);

typedef struct nimble_meshconn_eventcb_entry {
    list_node_t node;
    nimble_meshconn_eventcb_t cb;
} nimble_meshconn_sub_t;

typedef struct {
    // uint32_t netid;
    // uint16_t role;
    uint16_t notconn_scan_itvl;
    uint16_t notconn_scan_win;
    uint16_t notconn_timeout;

    uint16_t reconn_scan_itvl;
    uint16_t reconn_scan_win;
    uint16_t reconn_adv_itvl;
    uint32_t reconn_timeout;

    uint16_t grounded_scan_itvl;
    uint16_t grounded_scan_win;
    uint16_t grounded_adv_itvl;

    uint16_t conn_itvl_min;
    uint16_t conn_itvl_max;
    uint16_t conn_slave_latency;
    uint16_t conn_supervision_to;
    uint16_t conn_timeout;

    uint8_t links_up_min;
    uint8_t links_up_max;
} nimble_meshconn_params_t;

typedef struct __attribute__((packed)) {
    uint8_t uuid[sizeof(uint16_t)];
    uint8_t netid[sizeof(uint32_t)];
    uint8_t of_type;
    // uint8_t of_len;
    uint8_t of[sizeof(uint32_t)];
} nimble_meshconn_ad_t;


extern list_node_t nimble_meshconn_subs;

int nimble_meshconn_init(void);


static inline void nimble_meshconn_subscribe(nimble_meshconn_sub_t *sub)
{
    assert(sub);
    list_add(&nimble_meshconn_subs, &sub->node);
}

static inline int nimble_meshconn_unsubscribe(nimble_meshconn_sub_t *sub)
{
    assert(sub);
    return (list_remove(&nimble_meshconn_subs, &sub->node)) ? 0 : -ENOENT;
}


int nimble_meshconn_params(const nimble_meshconn_params_t *params);

int nimble_meshconn_create(uint32_t netid);

int nimble_meshconn_join(uint32_t netid);

int nimble_meshconn_leave(void);

void nimble_meshconn_updated_of(void);

#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_STATCONN_H */
/** @} */
