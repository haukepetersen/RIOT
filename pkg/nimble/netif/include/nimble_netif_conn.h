/*
 * Copyright (C) 2018-2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    pkg_nimble_netif_conn Nimble Netif Connection Allocation
 * @ingroup     pkg_nimble_netif
 * @brief       TODO
 * @{
 *
 * @file
 * @brief       Connection allocation and maintenance for Nimble netif
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef PKG_NIMBLE_NETIF_CONN_H
#define PKG_NIMBLE_NETIF_CONN_H

#include <stdint.h>

#include "nimble_netif.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NIMBLE_NETIF_CONN_INVALID       (-1)

#define NIMBLE_NETIF_CONN_NUMOF         MYNEWT_VAL_BLE_MAX_CONNECTIONS

/**
 * @brief   TODO
 */
typedef struct {
    struct ble_l2cap_chan *coc;
    uint16_t gaphandle;
    uint16_t state;
    uint8_t addr[BLE_ADDR_LEN];
} nimble_netif_conn_t;

typedef int (*nimble_netif_conn_iter_t)(nimble_netif_conn_t *conn,
                                        int handle, void *arg);

void nimble_netif_conn_init(void);

nimble_netif_conn_t *nimble_netif_conn_get(int handle);

int nimble_netif_conn_get_adv(void);

int nimble_netif_conn_get_connecting(void);

int nimble_netif_conn_get_by_addr(const uint8_t *addr);

int nimble_netif_conn_get_by_gaphandle(uint16_t gaphandle);

/* @note: everything done in the @p cb should be read only... */
void nimble_netif_conn_foreach(uint16_t filter,
                               nimble_netif_conn_iter_t cb, void *arg);

unsigned nimble_netif_conn_count(uint16_t filter);


int nimble_netif_conn_start_adv(void);

int nimble_netif_conn_start_connection(const uint8_t *addr);

void nimble_netif_conn_free(int handle);

static inline
nimble_netif_conn_t *nimble_netif_conn_from_gaphandle(uint16_t gh)
{
    return nimble_netif_conn_get(nimble_netif_conn_get_by_gaphandle(gh));
}

static inline int nimble_netif_conn_connecting(void)
{
    return (nimble_netif_conn_get_connecting() != NIMBLE_NETIF_CONN_INVALID);
}

static inline int nimble_netif_conn_connected(const uint8_t *addr)
{
    return (nimble_netif_conn_get_by_addr(addr) != NIMBLE_NETIF_CONN_INVALID);
}

static inline int nimble_netif_conn_is_adv(void)
{
    return (nimble_netif_conn_get_adv() != NIMBLE_NETIF_CONN_INVALID);
}

#ifdef __cplusplus
}
#endif

#endif /* PKG_NIMBLE_NETIF_CONN_H */
/** @} */
