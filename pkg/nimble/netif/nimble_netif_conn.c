/*
 * Copyright (C) 2018-2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkt_nimble_netif
 *
 * # TODO
 * - many many things...
 *
 *
 * @{
 *
 * @file
 * @brief       Connection context table handling for nimble netif
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "nimble_netif_conn.h"

static mutex_t _lock = MUTEX_INIT;
static nimble_netif_conn_t _conn[NIMBLE_NETIF_CONN_NUMOF];


static int _find_by_state(uint16_t filter)
{
    for (int i = 0; i < NIMBLE_NETIF_CONN_NUMOF; i++) {
        if (_conn[i].state & filter) {
            return i;
        }
    }
    return NIMBLE_NETIF_CONN_INVALID;
}

void nimble_netif_conn_init(void)
{
    memset(_conn, 0, sizeof(_conn));
    for (unsigned i = 0; i < NIMBLE_NETIF_CONN_NUMOF; i++) {
        _conn[i].state = NIMBLE_NETIF_UNUSED;
    }
}

nimble_netif_conn_t *nimble_netif_conn_get(int handle)
{
    if ((handle < 0) || (handle >= NIMBLE_NETIF_CONN_NUMOF)) {
        return NULL;
    }
    return &_conn[handle];
}

int nimble_netif_conn_get_adv(void)
{
    int handle;
    mutex_lock(&_lock);
    handle = _find_by_state(NIMBLE_NETIF_ADV);
    mutex_unlock(&_lock);
    return handle;
}

int nimble_netif_conn_get_by_addr(const uint8_t *addr)
{
    assert(addr);
    int handle = NIMBLE_NETIF_CONN_INVALID;

    mutex_lock(&_lock);
    for (unsigned i = 0; i < NIMBLE_NETIF_CONN_NUMOF; i++) {
        if ((_conn[i].state & NIMBLE_NETIF_L2CAP_CONNECTED) &&
            memcmp(_conn[i].addr, addr, BLE_ADDR_LEN) == 0) {
            handle = (int)i;
            break;
        }
    }
    mutex_unlock(&_lock);

    return handle;
}

int nimble_netif_conn_get_by_gaphandle(uint16_t gaphandle)
{
    int handle = NIMBLE_NETIF_CONN_INVALID;

    mutex_lock(&_lock);
    for (unsigned i = 0; i < NIMBLE_NETIF_CONN_NUMOF; i++) {
        if (_conn[i].gaphandle == gaphandle) {
            handle = (int)i;
            break;
        }
    }
    mutex_unlock(&_lock);

    return handle;
}

int nimble_netif_conn_start_connection(const uint8_t *addr)
{
    int handle;

    mutex_lock(&_lock);
    handle = _find_by_state(NIMBLE_NETIF_UNUSED);
    if (handle != NIMBLE_NETIF_CONN_INVALID) {
        _conn[handle].state = NIMBLE_NETIF_CONNECTING;
        memcpy(_conn[handle].addr, addr, BLE_ADDR_LEN);
    }

    mutex_unlock(&_lock);

    return handle;
}

int nimble_netif_conn_start_adv(void)
{
    int res = NIMBLE_NETIF_NOMEM;

    mutex_lock(&_lock);
    int handle = _find_by_state(NIMBLE_NETIF_ADV);
    if (handle != NIMBLE_NETIF_CONN_INVALID) {
        res = NIMBLE_NETIF_ALREADY;
    }
    else {
        handle = _find_by_state(NIMBLE_NETIF_UNUSED);
        if (handle != NIMBLE_NETIF_CONN_INVALID) {
            _conn[handle].state = NIMBLE_NETIF_ADV;
            res = NIMBLE_NETIF_OK;
        }

    }
    mutex_unlock(&_lock);

    return res;
}

void nimble_netif_conn_free(int handle)
{
    assert((handle > 0) && (handle < NIMBLE_NETIF_CONN_NUMOF));

    mutex_lock(&_lock);
    memset(&_conn[handle], 0, sizeof(nimble_netif_conn_t));
    _conn[handle].state = NIMBLE_NETIF_UNUSED;
    mutex_unlock(&_lock);
}

void nimble_netif_conn_foreach(uint16_t filter,
                               nimble_netif_conn_iter_t cb, void *arg)
{
    assert(cb);

    mutex_lock(&_lock);
    for (unsigned i = 0; i < NIMBLE_NETIF_CONN_NUMOF; i++) {
        if (_conn[i].state & filter) {
            int res = cb(&_conn[i], (int)i, arg);
            if (res != 0) {
                break;
            }
        }
    }
    mutex_unlock(&_lock);
}

unsigned nimble_netif_conn_count(uint16_t filter)
{
    unsigned cnt = 0;

    mutex_lock(&_lock);
    for (unsigned i = 0; i < NIMBLE_NETIF_CONN_NUMOF; i++) {
        if (_conn[i].state & filter) {
            ++cnt;
        }
    }
    mutex_unlock(&_lock);

    return cnt;
}
