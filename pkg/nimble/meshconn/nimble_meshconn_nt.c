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
#include "nimble_meshconn_of.h"
#include "nimble_netif_conn.h"
#include "nimble/nimble_port.h"


#define ENABLE_DEBUG    0
#include "debug.h"
// DEBUG
#include "net/bluetil/addr.h"

#if IS_USED(MODULE_MYPRINTuuu)
#include "myprint.h"
#else
#define myprintf(...)
#endif


static struct ble_npl_callout _nt_timeout_event;

static nimble_meshconn_sub_t _meshconn_sub;

static nimble_meshconn_nt_peer_t _nbr[NIMBLE_MESHCONN_NT_TABLE_SIZE];


static nimble_meshconn_nt_peer_t *_find_by_addr(const ble_addr_t *addr)
{
    for (int i = 0; i < ARRAY_SIZE(_nbr); i++) {
        if ((_nbr[i].state != NIMBLE_MESHCONN_NT_UNUSED) &&
            (memcmp(&_nbr[i].addr, addr, sizeof(_nbr[i].addr)) == 0)) {
            return &_nbr[i];
        }
    }
    return NULL;
}

static nimble_meshconn_nt_peer_t *_find_by_addr_plain(const uint8_t *addr)
{
    uint8_t addrn[BLE_ADDR_LEN];
    bluetil_addr_swapped_cp(addr, addrn);

    for (int i = 0; i < ARRAY_SIZE(_nbr); i++) {
        if ((_nbr[i].state != NIMBLE_MESHCONN_NT_UNUSED) &&
            (memcmp(&_nbr[i].addr.val, addrn, BLE_ADDR_LEN) == 0)) {
            return &_nbr[i];
        }
    }
    return NULL;
}

static nimble_meshconn_nt_peer_t *_find_highest_unused(uint32_t of_new)
{
    uint32_t of_highest = of_new;
    nimble_meshconn_nt_peer_t *nb = NULL;

    for (int i = 0; i < ARRAY_SIZE(_nbr); i++) {
        if (_nbr[i].state == NIMBLE_MESHCONN_NT_UNUSED) {
            return &_nbr[i];
        }
        else if ((_nbr[i].state == NIMBLE_MESHCONN_NT_NOTCONN) &&
                 (_nbr[i].of > of_highest)) {
            of_highest = _nbr[i].of;
            nb = &_nbr[i];
        }
    }

    return nb;
}

static void _on_cleanup_event(struct ble_npl_event *ev)
{
    (void)ev;

    uint32_t now = ztimer_now(ZTIMER_MSEC);

    for (int i = 0; i < ARRAY_SIZE(_nbr); i++) {
        if ((_nbr[i].state == NIMBLE_MESHCONN_NT_NOTCONN) &&
            ((_nbr[i].last_update + NIMBLE_MESHCONN_NT_TIMEOUT_MS) < now)) {
            _nbr[i].state = NIMBLE_MESHCONN_NT_UNUSED;
            myprintf("[nt] dropping entry %i\n", i);
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

    peer->state = state;

    if ((state == NIMBLE_MESHCONN_NT_UPLINK) ||
        (state == NIMBLE_MESHCONN_NT_DOWNLINK)) {
        peer->handle = handle;
    }
    else {
        peer->handle = NIMBLE_NETIF_CONN_INVALID;
    }

    // myprintf("[nt] update state for h:%i -> s:%i\n", handle, (int)state);
    // for (int i = 0; i < ARRAY_SIZE(_nbr); i++) {
    //     char str[BLUETIL_ADDR_STRLEN];
    //     uint8_t addrn[BLE_ADDR_LEN];
    //     bluetil_addr_swapped_cp(_nbr[i].addr.val, addrn);
    //     bluetil_addr_sprint(str, addrn);
    //     myprintf("%02i - %s h:%i of:%u", i, str, _nbr[i].handle, (unsigned)_nbr[i].of);
    // }
}

static void _on_meshconn_event(nimble_meshconn_event_t event, int handle,
                               const uint8_t *addr)
{
    switch (event) {
        case NIMBLE_MESHCONN_CONN_UPLINK:
            _update_state(addr, handle, NIMBLE_MESHCONN_NT_UPLINK);
            break;
        case NIMBLE_MESHCONN_CONN_DOWNLINK:
            _update_state(addr, handle, NIMBLE_MESHCONN_NT_DOWNLINK);
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
    memset(_nbr, 0, sizeof(_nbr));
    for (int i = 0; i < ARRAY_SIZE(_nbr); i++) {
        _nbr[i].state = NIMBLE_MESHCONN_NT_UNUSED;
        _nbr[i].handle = NIMBLE_NETIF_CONN_INVALID;
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
                                  uint8_t of_type, uint32_t of,
                                  bool connectable)
{
    /* only accept peers with a supported OF type */
    if (!nimble_meshconn_of_is_supported(of_type)) {
        return -1;
    }

    // DEBUG...
    char str[BLUETIL_ADDR_STRLEN];
    uint8_t addrn[BLE_ADDR_LEN];
    bluetil_addr_swapped_cp(addr->val, addrn);
    bluetil_addr_sprint(str, addrn);

    nimble_meshconn_nt_peer_t *nb = _find_by_addr(addr);
    if ((nb == NULL) && connectable) {
        nb = _find_highest_unused(of);
        if (nb != NULL) {
            memcpy(&nb->addr, addr, sizeof(*addr));
            nb->state = NIMBLE_MESHCONN_NT_NOTCONN;

            myprintf("[nt] insert %s of:%u\n", str, (unsigned)of);
        }
    }
    else {
        myprintf("[nt] update %s of:%u\n", str, (unsigned)of);
    }

    if (nb != NULL) {
        nb->rssi = rssi;
        nb->of_type = of_type;
        nb->of = of;
        nb->last_update = ztimer_now(ZTIMER_MSEC);
    }
    else {
        myprintf("[nt] discover: unable to add/update entry\n");
    }

    return 0;
}

nimble_meshconn_nt_peer_t *nimble_meshconn_nt_best(
                            nimble_meshconn_nt_state_t state, uint8_t of_type)
{
    nimble_meshconn_nt_peer_t *res = NULL;

    for (int i = 0; i < ARRAY_SIZE(_nbr); i++) {
        if ((_nbr[i].state == state) &&
            ((of_type == NIMBLE_MESHCONN_OF_NONE) || (_nbr[i].of_type = of_type)) &&
            ((res == NULL) || (_nbr[i].of < res->of))) {
            res = &_nbr[i];
        }
    }

    return res;
}

uint32_t nimble_meshconn_nt_peer_of(int handle)
{
    for (int i = 0; i < ARRAY_SIZE(_nbr); i++) {
        if ((_nbr[i].state != NIMBLE_MESHCONN_NT_UNUSED) &&
            (_nbr[i].handle == handle)) {

            char str[BLUETIL_ADDR_STRLEN];
            uint8_t addrn[BLE_ADDR_LEN];
            bluetil_addr_swapped_cp(_nbr[i].addr.val, addrn);
            bluetil_addr_sprint(str, addrn);
            myprintf("[nt] peer_of found: %s handle:%i slot:%i, of:%u\n",
                str, handle, i, (unsigned)_nbr[i].of);

            return _nbr[i].of;
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
    for (int i = 0; i < ARRAY_SIZE(_nbr); i++) {
        if (_nbr[i].handle != NIMBLE_NETIF_CONN_INVALID) {
            DEBUG("[nt] ERROR: purging table while still having open connections\n");
        }
        _nbr[i].of = 0;
        _nbr[i].handle = NIMBLE_NETIF_CONN_INVALID;
    }

    return 0;
}
