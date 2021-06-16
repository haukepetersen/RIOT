/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble_meshconn
 * @{
 *
 * @file
 * @brief       Statconn - static connection manager for NimBLE netif
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <errno.h>

#include "ztimer.h"
#include "nimble_meshconn.h"
#include "nimble_meshconn_nt.h"
#include "nimble_netif_conn.h"
#include "nimble/nimble_port.h"


#define ENABLE_DEBUG    1
#include "debug.h"

// DEBUG
#include "net/bluetil/addr.h"


static struct ble_npl_callout _nt_timeout_event;

static nimble_meshconn_sub_t _meshconn_sub;



// TODO: outsource to separate file
/* access: nimble thread only */
static struct {
    nimble_meshconn_nt_peer_t tab[NIMBLE_MESHCONN_NT_TABLE_SIZE];
    int high;
    int low;
    uint8_t of_type;
} _nt;


static nimble_meshconn_nt_peer_t *_find_by_addr(const ble_addr_t *addr)
{
    for (int i = 0; i < ARRAY_SIZE(_nt.tab); i++) {
        if ((_nt.tab[i].state != NIMBLE_MESHCONN_NT_UNUSED) &&
            (memcmp(&_nt.tab[i].addr, addr, sizeof(_nt.tab[i].addr)) == 0)) {
            return &_nt.tab[i];
        }
    }
    return NULL;
}

static nimble_meshconn_nt_peer_t *_find_by_addr_plain(const uint8_t *addr)
{
    uint8_t addrn[BLE_ADDR_LEN];
    bluetil_addr_swapped_cp(addr, addrn);

    for (int i = 0; i < ARRAY_SIZE(_nt.tab); i++) {
        if ((_nt.tab[i].state != NIMBLE_MESHCONN_NT_UNUSED) &&
            (memcmp(&_nt.tab[i].addr.val, addrn, BLE_ADDR_LEN) == 0)) {
            return &_nt.tab[i];
        }
    }
    return NULL;
}

static void _nt_set_limits(void)
{
    uint32_t of_low = UINT32_MAX;
    uint32_t of_high = 0;

    for (int i = 0; i < ARRAY_SIZE(_nt.tab); i++) {
        if (_nt.tab[i].of < of_low) {
            _nt.low = i;
        }
        if (_nt.tab[i].of > of_high) {
            _nt.high = i;
        }
    }
}

static void _insert(const ble_addr_t *addr, uint8_t rssi,
                    uint8_t of_type, uint32_t of)
{

    if (of <= _nt.tab[_nt.low].of) {
        return;
    }

    DEBUG("[nt] insert of_type:%u of:%u addr:", (unsigned)of_type, (unsigned)of);
    bluetil_addr_print(addr->val);
    DEBUG("\n");

    memcpy(&_nt.tab[_nt.low].addr, addr, sizeof(_nt.tab[_nt.low].addr));
    _nt.tab[_nt.low].state = NIMBLE_MESHCONN_NT_NOTCONN;
    _nt.tab[_nt.low].rssi = rssi;
    _nt.tab[_nt.low].of_type = of_type;
    _nt.tab[_nt.low].of = of;
    _nt.tab[_nt.low].last_update = ztimer_now(ZTIMER_MSEC);
    _nt_set_limits();
}

static void _on_cleanup_event(struct ble_npl_event *ev)
{
    (void)ev;

    uint32_t last_valid = ztimer_now(ZTIMER_MSEC) - NIMBLE_MESHCONN_NT_TIMEOUT_MS;

    for (int i = 0; i < ARRAY_SIZE(_nt.tab); i++) {
        if ((_nt.tab[i].of > 0) && (_nt.tab[i].last_update < last_valid)) {
            memset(&_nt.tab[i].addr, 0, sizeof(_nt.tab[i].addr));
            _nt.tab[i].of = 0;
            _nt.tab[i].state = NIMBLE_MESHCONN_NT_UNUSED;
            DEBUG("[nt] removing %i\n", i);
        }
    }

    ble_npl_callout_reset(&_nt_timeout_event, NIMBLE_MESHCONN_NT_TIMEOUT_MS);
}

static void _update_state(const uint8_t *addr, int handle,
                  nimble_meshconn_nt_state_t state)
{

    nimble_meshconn_nt_peer_t *peer = _find_by_addr_plain(addr);
    if (peer == NULL) {
        DEBUG("[nt] _update_state: error, peer not found\n");
        return;
    }

    peer->handle = handle;     // TODO: only set handle on CONNECT event
    peer->state = state;
}

static void _on_meshconn_event(nimble_meshconn_event_t event, int handle,
                               const uint8_t *addr)
{
    switch (event) {
        case NIMBLE_MESHCONN_CONN_UPLINK:
            _update_state(addr, handle, NIMBLE_MESHCONN_NT_UPSTREAM);
            break;
        case NIMBLE_MESHCONN_CONN_DOWNLINK:
            _update_state(addr, handle, NIMBLE_MESHCONN_NT_DOWNSTREAM);
            break;
        case NIMBLE_MESHCONN_CLOSE_UPLINK:
        case NIMBLE_MESHCONN_CLOSE_DOWNLINK:
            _update_state(addr, handle, NIMBLE_MESHCONN_NT_NOTCONN);
            break;
        default:
            /* do nothing */
            break;
    }
}

int nimble_meshconn_nt_init(void)
{
    memset(&_nt, 0, sizeof(_nt));
    for (int i = 0; i < ARRAY_SIZE(_nt.tab); i++) {
        _nt.tab[i].handle = NIMBLE_NETIF_CONN_INVALID;
    }

    /* subscribe to meshconn events */
    _meshconn_sub.cb = _on_meshconn_event;
    nimble_meshconn_subscribe(&_meshconn_sub);

    ble_npl_callout_init(&_nt_timeout_event, nimble_port_get_dflt_eventq(),
                         _on_cleanup_event, NULL);
    ble_npl_callout_reset(&_nt_timeout_event, NIMBLE_MESHCONN_NT_TIMEOUT_MS);

    return 0;
}

int nimble_meshconn_nt_discovered(const ble_addr_t *addr, int rssi,
                                  uint8_t of_type, uint32_t of)
{
    nimble_meshconn_nt_peer_t *e = _find_by_addr(addr);
    if (e != NULL) {
        e->rssi = rssi;
        e->of_type = of_type;
        e->of = of;
        e->last_update = ztimer_now(ZTIMER_MSEC);
        // DEBUG("[nt] disc update\n");
    }
    else {
        _insert(addr, rssi, of_type, of);
    }

    return 0;
}

nimble_meshconn_nt_peer_t *nimble_meshconn_nt_best(nimble_meshconn_nt_state_t state)
{
    uint32_t of_best = 0;
    nimble_meshconn_nt_peer_t *res = NULL;

    // DEBUG("[nt] best\n");
    for (int i = 0; i < ARRAY_SIZE(_nt.tab); i++) {
        // DEBUG("[nt] state:%i of:%u\n", (int)_nt.tab[i].state, (unsigned)_nt.tab[i].of);
        uint32_t of_cur = _nt.tab[i].of;
        if ((_nt.tab[i].state == state) &&
            (of_cur > of_best)) {
            of_best = of_cur;
            res = &_nt.tab[i];
        }
    }

    // DEBUG("[nt] best return %p\n", (void *)res);

    return res;
}

uint32_t nimble_meshconn_nt_peer_of(int handle)
{
    for (int i = 0; i < ARRAY_SIZE(_nt.tab); i++) {
        if (_nt.tab[i].handle == handle) {
            return _nt.tab[i].of;
        }
    }

    return 0;
}

/* CAUTION: this function is not thread save, make sure all other interactions
 *          with the neighbor table are disabled before calling this from a
 *          user thread
 */
int nimble_meshconn_nt_purge(void)
{
    for (int i = 0; i < ARRAY_SIZE(_nt.tab); i++) {
        memset(&_nt.tab[i].addr, 0, sizeof(_nt.tab[i].addr));
        _nt.tab[i].of = 0;
    }

    return 0;
}
