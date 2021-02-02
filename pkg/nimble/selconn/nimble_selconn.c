/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble_statconn
 * @{
 *
 * @file
 * @brief       Statconn - static connection manager for NimBLE netif
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "assert.h"
#include "net/bluetil/addr.h"
#include "net/bluetil/ad.h"
#include "nimble_netif.h"
#include "nimble_netif_conn.h"
#include "nimble_selconn.h"

#include "host/ble_hs.h"

#define ENABLE_DEBUG    0
#include "debug.h"


#define PEERS_MAXNUM        NIMBLE_NETIF_MAX_CONN

static const uint8_t _adfilter[] = { 0xfe, 0xaf, 0x01, 0x02, 0x03, 0x04 };

typedef struct {
    ble_addr_t addr;
    int handle;
} peer_t;

static struct {
    struct ble_gap_conn_params conn_params;
    uint32_t conn_timeout_ms;
    uint8_t peer_cnt;
    peer_t peers[PEERS_MAXNUM];
    uint32_t peering_timeout_ms;
    static struct ble_npl_callout peering_timeout_evt;
} _state;

static peer_t *_peer_get(const ble_addr_t *addr)
{
    peer_t *res = NULL;

    mutex_lock(&_state.lock);
    for (unsigned i = 0; i < PEERS_MAXNUM; i++) {
        if (memcmp(_state.peers[i].addr, addr, sizeof(ble_addr_t)) == 0) {
            res = &_state.peers[i];
            break;
        }
    }
    mutex_unlock(&_state.lock);

    return res;
}

static void _start_scan(void)
{
    mutex_lock(&_state.lock);
    for (unsigned i = 0; i < PEERS_MAXNUM; i++) {
        if (_state.peers[i].handle == NIMBLE_NETIF_CONN_INVALID) {
            nimble_scanner_init(&_state.background_timings, _on_background_scan_evt);
            nimble_scanner_start();
            return;
        }
    }
    mutex_unlock(&_state.lock);
}

static void _on_background_scan_evt(uint8_t type,
                                    const ble_addr_t *addr, int8_t rssi,
                                    const uint8_t *ad, size_t ad_len)
{
    (void)type;
    (void)rssi;
    (void)ad;
    (void)ad_len;

    peer_t *peer = _peer_get(addr);
    if (peer) {
        if (peer_handle == NIMBLE_NETIF_CONN_INVALID) {
            nimble_scanner_stop();
            nimble_netif_connect(addr,
                                 _state.conn_params, _state.conn_timeout_ms);
        }
    }
}

#if IS_ACTIVE(MODULE_NIMBLE_SELCONN_ROUTER)
static void _on_peering_scan_evt(uint8_t type,
                                 const ble_addr_t *addr, int8_t rssi,
                                 const uint8_t *ad, size_t ad_len)
{
    (void)type;
    (void)

    /* filter peers advertising data */
    bluetil_ad_t data;
    bluetil_ad_init(&data, ad, ad_len, ad_len);
    if (bluetil_ad_find_and_cmp(&data, BLE_GAP_AD_VENDOR,
                                _adfilter, sizeof(_adfilter)) == BLUETIL_AD_OK) {
        /* on success, add */
        _peer_add(addr);
        nimble_scanner_stop();
        _start_background_scan();
    }
}
#endif

void _on_peer_change(int handle,
                     nimble_netif_event_t event, const uint8_t *addr)
{

}

void _on_peering_end(void)
{

    // do nothing
    _start_background_scan();
}

void nimble_selconn_init(void)
{

    /* initialize the discovery timer */

    /* register event callbacks */
    nimble_scanner_init(XX, _on_scan_result);
    nimble_netif_eventcb(_on_peer_change);

    ble_npl_callout_init(&_state.peering_timeout_evt,
                         nimble_port_get_dflt_eventq(),
                         _on_peering_end, NULL);
}

int nimble_selconn_discover(uint32_t timeout_ms)
{
    if (_state.peers == PEERS_MAXNUM) {
        return -ENOMEM;
    }

    ble_npl_callout_reset(&_state.peering_timeout_evt, _state.peering_timeout_ms);
    nimble_scanner_init(&_state.peering_timings, _on_peering_scan_evt);
    nimble_scanner_start();
}
