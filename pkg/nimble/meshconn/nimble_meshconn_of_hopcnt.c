/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble_meshconn_of
 * @{
 *
 * @file
 * @brief       Meshconn objective function: ICMP echo (ping)
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <errno.h>
#include <limits.h>



#include "nimble_meshconn.h"
#include "nimble_meshconn_of.h"
#include "nimble_meshconn_nt.h"

#define ENABLE_DEBUG    1
#include "debug.h"


static nimble_meshconn_of_t _of;
static nimble_meshconn_sub_t _meshconn_sub;
static uint32_t _val = 0;


static void _update(void)
{
    int of_new;

    nimble_meshconn_nt_peer_t *peer = nimble_meshconn_nt_best(NIMBLE_MESHCONN_NT_UPSTREAM);
    if (peer == 0) {
        of_new = 0;
    }
    else {
        of_new = peer->of + 1;
    }

    if (of_new != _val) {
        _val = of_new;
        nimble_meshconn_of_notify(NIMBLE_MESHCONN_OF_HOPCNT, of_new);
    }
}

static void _on_meshconn_event(nimble_meshconn_event_t event, int handle,
                               const uint8_t *addr)
{
    (void)handle;
    (void)addr;

    DEBUG("[of_hop] mc event:%i\n", (int)event);

    switch (event) {
        case NIMBLE_MESHCONN_CONN_UPLINK:
        case NIMBLE_MESHCONN_CLOSE_UPLINK:
            _update();
            break;
        default:
            /* do nothing */
            break;
    }
}

static int _activate(uint8_t role)
{
    DEBUG("[of_hop] activated, role:%u\n", (unsigned)role);

    _meshconn_sub.cb = _on_meshconn_event;
    nimble_meshconn_subscribe(&_meshconn_sub);

    if (role != 0) {
        _val = 0;
    }
    else {
        _val = 1;
        nimble_meshconn_of_notify(NIMBLE_MESHCONN_OF_HOPCNT, _val);
    }

    return 0;
}

static int _deactivate(void)
{
    DEBUG("[of_hop] deactivated\n");

    nimble_meshconn_unsubscribe(&_meshconn_sub);

    return 0;
}


int nimble_meshconn_of_hopcnt_init(void)
{
    int res;
    (void)res;

    /* register this OF */
    _of.activate = _activate;
    _of.deactivate = _deactivate;
    res = nimble_meshconn_of_register(NIMBLE_MESHCONN_OF_HOPCNT, &_of);

    return 0;
}
