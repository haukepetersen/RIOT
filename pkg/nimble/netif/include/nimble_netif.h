/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    pkg_nimble_netif
 * @ingroup     pkg_nimble
 * @brief       TODO
 * @{
 *
 * @file
 * @brief       GNRC integration for NimBLE
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef PKG_NIMBLE_NETIF_H
#define PKG_NIMBLE_NETIF_H

#include <stdint.h>

#include "clist.h"
#include "net/ble.h"

#include "host/ble_hs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NIMBLE_NETIF_CID
#define NIMBLE_NETIF_CID            (BLE_L2CAP_CID_IPSP)
#endif

enum {
    NIMBLE_NETIF_OK      =  0,
    NIMBLE_NETIF_NOTCONN = -1,
    NIMBLE_NETIF_DEVERR  = -2,
    NIMBLE_NETIF_BUSY    = -3,
    NIMBLE_NETIF_NOMEM   = -4,
    NIMBLE_NETIF_NOTADV  = -5,
    NIMBLE_NETIF_ALREADY = -6,
};

typedef enum {
    NIMBLE_NETIF_CONNECTED,
    NIMBLE_NETIF_DISCONNECTED,
    NIMBLE_NETIF_ABORT,
    NIMBLE_NETIF_CONN_UPDATED,
} nimble_netif_event_t;


/* connection states: DISCONNECTED, BUSY, ADVERTISING, CONNECTED
 * -> how do we know the state from the value of the fields of this struct? */
typedef struct {
    clist_node_t node;      /* could we use this value to test the state? */
    uint8_t addr[BLE_ADDR_LEN];
    uint8_t role;
    struct ble_l2cap_chan *coc;
    uint16_t handle;    /* needed? -> maybe better use state? */
} nimble_netif_conn_t;


typedef void(*nimble_netif_eventcb_t)(nimble_netif_conn_t *conn,
                                      nimble_netif_event_t event);


/* to be called from: system init (auto_init) */
void nimble_netif_init(void);

/**
 * @brief   Register a global event callback, servicing all NimBLE Imps
 *
 * @warning     This function **must** be called before and other action
 *
 * @params[in] cb       event callback to register, may be NULL
 */
// TODO: move this into the _init() function? But what about auto_init?
int nimble_netif_eventcb(nimble_netif_eventcb_t cb);


int nimble_netif_connect(nimble_netif_conn_t *conn,
                         const ble_addr_t *addr,
                         const struct ble_gap_conn_params *conn_params,
                         uint32_t connect_timeout);

/* return: negative for fail */
int nimble_netif_accept(nimble_netif_conn_t *conn,
                        const uint8_t *ad, size_t ad_len,
                        const struct ble_gap_adv_params *adv_params);

int nimble_netif_update(nimble_netif_conn_t,
                        struct ble_gap_conn_params *conn_params);

int nimble_netif_terminate(nimble_netif_conn_t *conn);

int nimble_netif_is_connected(nimble_netif_conn_t *conn);

int nimble_netif_is_connected_addr(const ble_addr_t *addr);

nimble_netif_conn_t *nimble_netif_get_conn(const uint8_t *addr);

nimble_netif_conn_t *nimble_netif_get_adv_ctx(void);

void nimble_netif_print_conn_info(void);

#ifdef __cplusplus
}
#endif

#endif /* PKG_NIMBLE_NETIF_H */
/** @} */
