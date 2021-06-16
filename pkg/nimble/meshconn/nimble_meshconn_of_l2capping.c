/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble_meshconn_of_l2capping
 * @{
 *
 * @file
 * @brief       Meshconn objective function: L2CAP echo (ping)
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <errno.h>
#include <limits.h>

#include "assert.h"
#include "nimble/nimble_port.h"
#include "nimble_netif_conn.h"

#include "nimble_meshconn.h"
#include "nimble_meshconn_of.h"
#include "nimble_meshconn_nt.h"

#define TIMEOUT_INIT            250     // TODO: make dependent on conn itvl
#define TIMEOUT_PROBE           5000    // TODO: make dependent on conn itvl

#define INIT_SAMPLES            20
#define NB_LEN                  NIMBLE_NETIF_MAX_CONN

#define ENABLE_DEBUG    1
#include "debug.h"

static nimble_meshconn_of_t _of;
static nimble_meshconn_sub_t _meshconn_sub;

static uint32_t _val = 0;
static int _handle = -1;

struct {
    uint32_t rtt;
    int state;
    struct ble_npl_callout probe_timer;
} _peers[NB_LEN] = { 0 };


static void _update(int handle)
{
    DEBUG("[of_l2capping] _update h:%i, old:%u rtt:%u\n",
            handle, (unsigned)_val, (unsigned)_peers[handle].rtt);

    uint32_t base = nimble_meshconn_nt_peer_of(handle);
    if (base == 0) {
        return;
    }

    uint32_t of = base + _peers[handle].rtt;
    DEBUG("[of_l2capping] new of:%u\n", (unsigned)of);
    if ((_val == 0) || (of < _val) || ((_handle == handle) && _val != of)) {
        _val = of;
        _handle = handle;
        nimble_meshconn_of_notify(NIMBLE_MESHCONN_OF_L2CAPPING, _val);
    }
}

static void _on_echo_rsp(uint16_t gap_handle, uint32_t rtt_ms,
                         struct os_mbuf *om)
{
    int handle = nimble_netif_conn_get_by_gaphandle(gap_handle);
    if (handle == NIMBLE_NETIF_CONN_INVALID) {
        DEBUG("[of_l2capping] INVALID GAP HANDLE on echo response!\n");
        return;
    }

    if (_peers[handle].state == INIT_SAMPLES) {
        _peers[handle].rtt = (((_peers[handle].rtt * INIT_SAMPLES) + rtt_ms) /
                               (INIT_SAMPLES + 1));
        _update(handle);
    }
    else if (_peers[handle].state < INIT_SAMPLES) {
        _peers[handle].state ++;
        _peers[handle].rtt += rtt_ms;

        /* if this was the last sample, this peer is initialized and we include
         * it into consideration when calculating our own objective function
         * value */
        if (_peers[handle].state == INIT_SAMPLES) {
            _peers[handle].rtt /= INIT_SAMPLES;
            DEBUG("[of_l2capping] init probing done, rtt:%u\n", (unsigned)_peers[handle].rtt);
            _update(handle);
        }

    }
}

static void _on_probe_timeout(struct ble_npl_event *ev)
{
    int handle = (int)ev->arg;

    if (_peers[handle].state == 0) {
        return;
    }

    nimble_netif_l2cap_ping(handle, _on_echo_rsp, NULL, 0);

    // TODO: timeout depending on connection interval...
    uint16_t conn_itvl = nimble_netif_conn_get_conn_itvl(handle);
    uint32_t timeout = (_peers[handle].state < INIT_SAMPLES) ? ((11 * conn_itvl) / 10)
                                                             : ((301 * conn_itvl) / 10);
    ble_npl_callout_reset(&_peers[handle].probe_timer, timeout);
}

static void _uplink_conn(int handle)
{
    DEBUG("[of_l2capping] uplink_conn, starting init timer\n");
    _peers[handle].rtt = 0;
    _peers[handle].state = 1;

    uint16_t conn_itvl = nimble_netif_conn_get_conn_itvl(handle);

    nimble_netif_l2cap_ping(handle, _on_echo_rsp, NULL, 0);
    ble_npl_callout_reset(&_peers[handle].probe_timer, ((11 * conn_itvl) / 10));
}

static void _uplink_down(int handle)
{
    ble_npl_callout_stop(&_peers[handle].probe_timer);
    _peers[handle].state = 0;

    /* see if there is any other upstream link that can be used */
    _val = 0;
    for (int i = 0; i < NB_LEN; i++) {
        if (_peers[i].state == INIT_SAMPLES) {
            _update(i);
        }
    }
    if (_val == 0) {
        nimble_meshconn_of_notify(0, 0);
    }
}

static void _on_meshconn_event(nimble_meshconn_event_t event, int handle,
                               const uint8_t *addr)
{
    (void)addr;

    DEBUG("[of_l2capping] mc event:%i\n", (int)event);

    switch (event) {
        case NIMBLE_MESHCONN_CONN_UPLINK:
            _uplink_conn(handle);
            break;
        case NIMBLE_MESHCONN_CLOSE_UPLINK:
            _uplink_down(handle);
            break;
        default:
            /* do nothing */
            break;
    }
}

static int _activate(uint8_t role)
{
    DEBUG("[of_l2capping] activated, role:%u\n", (unsigned)role);

    _meshconn_sub.cb = _on_meshconn_event;
    nimble_meshconn_subscribe(&_meshconn_sub);

    // TODO: specify ROLE
    if (role != 0) {
        _val = 0;
    }
    else {
        _val = 1;
        nimble_meshconn_of_notify(NIMBLE_MESHCONN_OF_L2CAPPING, _val);
    }

    return 0;
}

static int _deactivate(void)
{
    DEBUG("[of_l2capping] deactivated\n");
    nimble_meshconn_unsubscribe(&_meshconn_sub);

    return 0;
}


int nimble_meshconn_of_l2capping_init(void)
{
    int res;
    (void)res;

    for (int i = 0; i < NB_LEN; i++) {
        ble_npl_callout_init(&_peers[i].probe_timer,
                            nimble_port_get_dflt_eventq(),
                            _on_probe_timeout, (void *)i);
    }


    /* register this OF */
    _of.activate = _activate;
    _of.deactivate = _deactivate;
    res = nimble_meshconn_of_register(NIMBLE_MESHCONN_OF_L2CAPPING, &_of);

    return 0;
}
