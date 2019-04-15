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
    NIMBLE_NETIF_CONNECTED_MASTER,
    NIMBLE_NETIF_CONNECTED_SLAVE,
    NIMBLE_NETIF_DISCONNECTED,
    NIMBLE_NETIF_CONNECT_ABORT,
    NIMBLE_NETIF_ACCEPT_ABORT,
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
 * @brief   Register a global event callback, servicing all NimBLE connections
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

int nimble_netif_disconnect(nimble_netif_conn_t *conn);

/* return: negative for fail */
int nimble_netif_accept(nimble_netif_conn_t *conn,
                        const uint8_t *ad, size_t ad_len,
                        const struct ble_gap_adv_params *adv_params);

/**
 * @brief   Stop accepting incoming connections (and stop advertising)
 *
 * If an advertising context was set, this function will trigger a
 * NIMBLE_NETIF_ACCEPT_ABORT event.
 *
 * @return  NIMBLE_NETIF_OK on success
 * @return  NIMBLE_NETIF_NOTADV if no advertising context is set
 */
int nimble_netif_accept_stop(void);

// /**
//  * @brief   Pause accepting incoming connections (stop sending advertisements)
//  *
//  * @return  NIMBLE_NETIF_OK on success
//  * @return  NIMBLE_NETIF_NOTADV if no advertising context is set
//  */
// int nimble_netif_accept_pause(void);

// *
//  * @brief   Resume accepting connections
//  *
//  * The context set by nimble_netif_accept() will be used.
//  *
//  * @return  NIMBLE_NETIF_OK on success
//  * @return  NIMBLE_NETIF_BUSY if we are already accepting connections
//  * @return  NIMBLE_NETIF_NOTADV if no advertising context is set

// int nimble_netif_accept_resume(void);

/**
 * @brief   Update the connection parameters for the given connection
 *
 * @param t [description]
 * @param ble_gap_conn_params [description]
 *
 * @return [description]
 */
int nimble_netif_update(nimble_netif_conn_t *conn,
                        struct ble_gap_conn_params *conn_params);

/**
 * @brief   Get the connection context corresponding to the given address
 *
 * @return  connection context for the connection to the given address
 * @return  NULL if not connected to the node with the given address
 */
nimble_netif_conn_t *nimble_netif_get_conn(const uint8_t *addr);

/* deprecated? */
// int nimble_netif_is_connected(nimble_netif_conn_t *conn);

/* deprecated? */
// void nimble_netif_print_conn_info(void);

#ifdef __cplusplus
}
#endif

#endif /* PKG_NIMBLE_NETIF_H */
/** @} */
