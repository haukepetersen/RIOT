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
    NIMBLE_NETIF_OK         =  0,
    NIMBLE_NETIF_NOTCONN    = -1,
    NIMBLE_NETIF_DEVERR     = -2,
    NIMBLE_NETIF_BUSY       = -3,
    NIMBLE_NETIF_NOMEM      = -4,
    NIMBLE_NETIF_NOTADV     = -5,
    NIMBLE_NETIF_ALREADY    = -6,
    NIMBLE_NETIF_NOTFOUND   = -7,
};

typedef enum {
    NIMBLE_NETIF_CONNECTED_MASTER,
    NIMBLE_NETIF_CONNECTED_SLAVE,
    NIMBLE_NETIF_DISCONNECTED,
    NIMBLE_NETIF_CONNECT_ABORT,
    NIMBLE_NETIF_CONN_UPDATED,
} nimble_netif_event_t;

enum {
    NIMBLE_NETIF_L2CAP_CLIENT       = 0x0001,
    NIMBLE_NETIF_L2CAP_SERVER       = 0x0002,
    NIMBLE_NETIF_L2CAP_CONNECTED    = 0x0003,
    NIMBLE_NETIF_GAP_MASTER         = 0x0010,
    NIMBLE_NETIF_GAP_SLAVE          = 0x0020,
    NIMBLE_NETIF_GAP_CONNECTED      = 0x0030,
    NIMBLE_NETIF_ADV                = 0x0100,
    NIMBLE_NETIF_CONNECTING         = 0x4000,
    NIMBLE_NETIF_UNUSED             = 0x8000,
};

typedef struct {
    struct ble_l2cap_chan *coc;
    uint16_t gaphandle;
    uint16_t state;
    uint8_t addr[BLE_ADDR_LEN];
} nimble_netif_conn_t;


typedef void(*nimble_netif_eventcb_t)(int handle, nimble_netif_event_t event);


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


int nimble_netif_connect(const ble_addr_t *addr,
                         const struct ble_gap_conn_params *conn_params,
                         uint32_t connect_timeout);

int nimble_netif_close(int handle);

/* return: negative for fail */
int nimble_netif_accept(const uint8_t *ad, size_t ad_len,
                        const struct ble_gap_adv_params *adv_params);

/**
 * @brief   Stop accepting incoming connections (and stop advertising)
 * *
 * @return  NIMBLE_NETIF_OK on success
 * @return  NIMBLE_NETIF_NOTADV if no advertising context is set
 */
int nimble_netif_accept_stop(void);

/**
 * @brief   Update the connection parameters for the given connection
 *
 * @param t [description]
 * @param ble_gap_conn_params [description]
 *
 * @return [description]
 */
int nimble_netif_update(int handle, struct ble_gap_conn_params *conn_params);



/**
 * @brief   Get the connection context corresponding to the given address
 *
 * @return  connection context for the connection to the given address
 * @return  NULL if not connected to the node with the given address
 */
// nimble_netif_conn_t *nimble_netif_get_conn(const uint8_t *addr);
/* deprecated? */
// int nimble_netif_is_connected(nimble_netif_conn_t *conn);

/* deprecated? */
// void nimble_netif_print_conn_info(void);

#ifdef __cplusplus
}
#endif

#endif /* PKG_NIMBLE_NETIF_H */
/** @} */
