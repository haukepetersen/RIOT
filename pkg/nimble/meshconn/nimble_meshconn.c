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

#include <limits.h>

#include "assert.h"
#include "byteorder.h"
#include "net/bluetil/ad.h"

#include "nimble_netif.h"
#include "nimble_netif_conn.h"
#include "nimble_meshconn.h"
#include "nimble_scanner.h"

#include "nimble/nimble_port.h"
#include "host/ble_hs.h"

#include "nimble_meshconn_nt.h"
#include "nimble_meshconn_of.h"

// REMOVE
#include "net/bluetil/addr.h"

#define ENABLE_DEBUG    1
#include "debug.h"

#define CONN_NUMOF              NIMBLE_NETIF_MAX_CONN

#define AD_TYPE                 BLE_GAP_AD_SERVICE_DATA
#define AD_UUID                 BLE_GATT_SVC_IPSS

// typedef struct {
//     uint8_t addr[BLE_ADDR_LEN];     /**< peer addr, network byte order */
//     uint8_t state;                  /**< internal state */
// } slot_t;

// static const uint8_t _ad[2] = { BLE_GAP_AD_FLAGS, BLUETIL_AD_FLAGS_DEFAULT };

// static mutex_t _lock = MUTEX_INIT;
// static slot_t _slots[NIMBLE_NETIF_MAX_CONN];

// static struct ble_gap_adv_params _adv_params;
// static struct ble_gap_conn_params _conn_params;
// static uint32_t _conn_timeout;

list_node_t nimble_meshconn_subs = { NULL };


// static slot_t *_get_addr(const uint8_t *addr)
// {
//     for (unsigned i = 0; i < ARRAY_SIZE(_slots); i++) {
//         if (memcmp(addr, _slots[i].addr, BLE_ADDR_LEN) == 0) {
//             return &_slots[i];
//         }
//     }
//     return NULL;
// }

// static slot_t *_get_state(uint8_t state)
// {
//     for (unsigned i = 0; i < ARRAY_SIZE(_slots); i++) {
//         if (_slots[i].state == state) {
//             return &_slots[i];
//         }
//     }
//     return NULL;
// }

enum {
    STATE_IDLE = 0,
    STATE_NOTCONN,
    STATE_GROUNDED,
    STATE_RECONN_UP,
    STATE_RECONN_DOWN,
    STATE_FLOATING,
};

enum {
    ROLE_BR = 0,
    ROLE_ROUTER,
    ROLE_NODE,
};

/* access: ANY! */
static struct {
    uint8_t role;
    uint8_t state;
    nimble_meshconn_params_t params;
    uint32_t id;
    uint8_t of_type;
    uint32_t of;
    struct {
        uint8_t up;
        uint8_t down;
    } link_cnt;
} _net = { 0 };

/* used events and timers */
static struct ble_npl_callout _connect_event;
static nimble_meshconn_of_sub_t _of_sub;


static void _notify(nimble_meshconn_event_t event,
                    int handle, const uint8_t *addr)
{
    list_node_t *node = nimble_meshconn_subs.next;
    while (node) {
        nimble_meshconn_sub_t *sub = (nimble_meshconn_sub_t *)node;
        sub->cb(event, handle, addr);
        node = node->next;
    }
}

static void _on_scan_evt(uint8_t type,
                         const ble_addr_t *addr, int8_t rssi,
                         const uint8_t *adbuf, size_t adbuf_len);

/* context: nimble thread only */
// static int _of_type_supported(uint8_t of_type)
// {
//     if (of_type == 7) {
//         return 1;
//     }
//     return 0;
// }

static void _get_addr(const uint8_t *addr_be, ble_addr_t *addr_le)
{
    // TODO: fix addr type!
    bluetil_addr_swapped_cp(addr_be, addr_le->val);
    addr_le->type = BLE_OWN_ADDR_RANDOM;
}

static void _connect(const ble_addr_t *addr,
                     uint16_t itvl, uint16_t win, uint32_t timeout)
{
    struct ble_gap_conn_params p = { 0 };
    p.scan_itvl = BLE_GAP_SCAN_ITVL_MS(itvl);
    p.scan_window = BLE_GAP_SCAN_WIN_MS(win);
    p.itvl_min = BLE_GAP_CONN_ITVL_MS(_net.params.conn_itvl_min);
    p.itvl_max = BLE_GAP_CONN_ITVL_MS(_net.params.conn_itvl_max);
    p.latency = _net.params.conn_slave_latency;
    p.supervision_timeout = BLE_GAP_SUPERVISION_TIMEOUT_MS(_net.params.conn_supervision_to);

    nimble_netif_connect(addr, &p, timeout);
}

static void _do_accept(void)
{
    int res;

    // start to send advertising, IF:
    // - state == STATE_GROUNDED and
    // - slots available
    // TODO implement this condition

    // unsigned free = nimble_netif_count(NIMBLE_NETIF_UNUSED);
    if ((CONN_NUMOF - _net.link_cnt.down) <= _net.params.links_up_min) {
        DEBUG("[mc] do_accept: skip because no free downstream slot available\n");
        return;
    }

    /* stop advertising (if ongoing) */
    nimble_netif_accept_stop();

    if (_net.of == 0) {
        return;
    }

    DEBUG("[mc] setting new OF:%u and start advertising\n", (unsigned)_net.of);
    /* generate advertising payload */
    nimble_meshconn_ad_t payload;
    byteorder_htolebufs(payload.uuid, AD_UUID);
    byteorder_htolebufl(payload.netid, _net.id);
    payload.of_type = _net.of_type;
    byteorder_htolebufl(payload.of, _net.of);

    uint8_t ad_buf[BLE_ADV_PDU_LEN];
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, ad_buf, sizeof(ad_buf), BLUETIL_AD_FLAGS_DEFAULT);
    res = bluetil_ad_add(&ad, AD_TYPE, &payload, sizeof(payload));
    assert(res == BLUETIL_AD_OK);

    /* start accepting incoming connections */
    // TODO: generalize to also work with extended advertisings
    struct ble_gap_adv_params p = { 0 };
    p.conn_mode = BLE_GAP_CONN_MODE_UND;
    p.disc_mode = BLE_GAP_DISC_MODE_GEN;
    p.itvl_min = BLE_GAP_ADV_ITVL_MS(_net.params.grounded_adv_itvl);
    p.itvl_max = BLE_GAP_ADV_ITVL_MS(_net.params.grounded_adv_itvl);
    res = nimble_netif_accept(ad.buf, ad.pos, &p);
    if (res != NIMBLE_NETIF_OK) {
        DEBUG("[mc] accpet err: %i\n", res);
    }
}

static void _do_connect(struct ble_npl_event *ev)
{
    (void)ev;

    DEBUG("[mc] connect event\n");

    /* must be true:
        - conn_upstream > 0
        -
    */
    nimble_meshconn_nt_peer_t *peer = nimble_meshconn_nt_best(NIMBLE_MESHCONN_NT_NOTCONN);
    if (peer) {
        nimble_scanner_stop();

        DEBUG("[mc] connnect now to ");
        bluetil_addr_print(peer->addr.val);
        DEBUG("\n");

        nimble_meshconn_of_activate(peer->of_type, 1);      // TODO role...
        _connect(&peer->addr,
                 _net.params.notconn_scan_itvl, _net.params.notconn_scan_win,
                 _net.params.conn_timeout);
    } else {
        ble_npl_callout_reset(&_connect_event, _net.params.notconn_timeout);
    }
}

static void _do_search(void)
{
    struct ble_gap_disc_params p = { 0 };
    p.itvl = BLE_GAP_SCAN_ITVL_MS(_net.params.notconn_scan_itvl);
    p.window = BLE_GAP_SCAN_WIN_MS(_net.params.notconn_scan_win);

    nimble_scanner_init(&p, _on_scan_evt);
    nimble_scanner_start();

    ble_npl_callout_reset(&_connect_event, _net.params.notconn_timeout);
}

static void _on_connected_up(int handle, const uint8_t *addr)
{
    _net.link_cnt.up++;
    DEBUG("[mc] on_connected_up - UP link cnt: %i\n", (int)_net.link_cnt.up);

    if (_net.state == STATE_RECONN_UP) {
        _net.state = STATE_GROUNDED;
        _notify(NIMBLE_MESHCONN_RECONNECT, handle, addr);
    }
    else {
        _notify(NIMBLE_MESHCONN_CONN_UPLINK, handle, addr);
    }

    /* start advertising our presence, if applicable */

    _do_accept();

    /* continue scanning with reduced duty cycle */
    struct ble_gap_disc_params p = { 0 };
    p.itvl = BLE_GAP_SCAN_ITVL_MS(_net.params.grounded_scan_itvl);
    p.window = BLE_GAP_SCAN_WIN_MS(_net.params.grounded_scan_win);
    nimble_scanner_start();

    /* continue searching for upstream nodes, with reduced interval */
    // TODO
}

static void _on_connected_down(int handle, const uint8_t *addr)
{
    _net.link_cnt.down++;
    DEBUG("[mc] on _connected_down - DOWN link cnt: %i\n", (int)_net.link_cnt.down);

    if (_net.state == STATE_RECONN_DOWN) {
        _net.state = STATE_GROUNDED;        // TODO: verify target state
        _notify(NIMBLE_MESHCONN_RECONNECT, handle, addr);
    }
    else {
        _notify(NIMBLE_MESHCONN_CONN_DOWNLINK, handle, addr);
    }

    _do_accept();
}

static void _on_closed_up(int handle, const uint8_t *addr)
{
    // TODO: make sure we are not in any other REPAIR state
    _net.link_cnt.up--;
    if (_net.state == STATE_IDLE) {
        _notify(NIMBLE_MESHCONN_CLOSE_UPLINK, handle, addr);
        DEBUG("[mc] on_closed_up -> IDLE\n");
        return;
    }

    DEBUG("[mc] on_closed_up -> RECONN_UP\n");

    /* try to reconnect right away */
    _net.state = STATE_RECONN_UP;
    ble_addr_t addr_le;
    _get_addr(addr, &addr_le);

    nimble_scanner_stop();
    nimble_netif_accept_stop();
    _connect(&addr_le, _net.params.reconn_scan_itvl, _net.params.reconn_scan_win,
             _net.params.reconn_timeout);

}

static void _on_closed_down(int handle, const uint8_t *addr)
{
    // TODO: differentiate state between upstream and downstream
    _net.link_cnt.down--;

    if (_net.state == STATE_IDLE) {
        _notify(NIMBLE_MESHCONN_CLOSE_DOWNLINK, handle, addr);
        DEBUG("[mc] on_closed_down -> IDLE\n");
        return;
    }

    DEBUG("[mc] on_closed_down -> RECONN_DOWN\n");

    /* try to reconnect instantly using directed advertising messages */
    // DEBUG("[RECONN] down\n");
    nimble_netif_accept_stop();
    _net.state = STATE_RECONN_DOWN;

    struct ble_gap_adv_params p = { 0 };
    p.conn_mode = BLE_GAP_CONN_MODE_DIR;    /* reconnect using directed adv */
    p.disc_mode = BLE_GAP_DISC_MODE_GEN;
    p.itvl_min = BLE_GAP_ADV_ITVL_MS(_net.params.reconn_adv_itvl);
    p.itvl_max = BLE_GAP_ADV_ITVL_MS(_net.params.reconn_adv_itvl);
    p.high_duty_cycle = 1;

    ble_addr_t addr_le;
    _get_addr(addr, &addr_le);
    nimble_netif_accept_direct(&addr_le, _net.params.reconn_timeout, &p);
}

static void _on_reconn_timeout(int handle, const uint8_t *addr)
{
    if (_net.state == STATE_RECONN_DOWN) {
        if ((_net.role == ROLE_BR) ||
            (_net.link_cnt.up > 0)) {
            _net.state = STATE_GROUNDED;
            _do_accept();
            DEBUG("[mc] on_reconn_timeout -> GROUNDED\n");
        }
        else {
            _net.state = STATE_NOTCONN;
            _do_search();
            DEBUG("[mc] on_reconn_timeout -> NOTCONN\n");
        }
        _notify(NIMBLE_MESHCONN_CLOSE_DOWNLINK, handle, addr);
    }
}

static void _on_abort_up(int handle, const uint8_t *addr)
{
    if (_net.state == STATE_RECONN_UP) {
        if (_net.link_cnt.up > 0) {
            _net.state = STATE_GROUNDED;
            DEBUG("[mc] on_abort_up -> GROUNDED\n");
        }
        else if (_net.link_cnt.down > 0) {
            _net.state = STATE_FLOATING;
            DEBUG("[mc] on_abort_up -> FLOATING\n");
            // TODO: initiate sequence to transision from FLOATING to NOTCONN
            //       or back to GROUNDED!
        }
        else {
            _net.state = STATE_NOTCONN;
            DEBUG("[mc] on_abort_up -> NOTCONN\n");
        }
        _notify(NIMBLE_MESHCONN_CLOSE_UPLINK, handle, addr);
    }

    /* in any case, continue to search a new uplink neighbor */
    _do_search();
}

static void _on_abort_down(void)
{
    DEBUG("[mc] on_abort_down -> X\n");
    _do_accept();
}

/* context: nimble thread only */
static void _on_scan_evt(uint8_t type,
                         const ble_addr_t *addr, int8_t rssi,
                         const uint8_t *adbuf, size_t adbuf_len)
{

    // TODO: extend to support EXT ADV
    if (type != BLE_HCI_ADV_TYPE_ADV_IND) {
        return;
    }

    bluetil_ad_t ad = {
        .buf = (uint8_t *)adbuf,
        .pos = adbuf_len,
        .size = adbuf_len,
    };

    /* make sure the advertising data contains anything of interest to us */
    bluetil_ad_data_t sd;
    if (bluetil_ad_find(&ad, AD_TYPE, &sd) == BLUETIL_AD_OK) {
        // DEBUG("[scan] is type:%u\n", (unsigned)AD_TYPE);
        uint16_t uuid = byteorder_lebuftohs(sd.data);
        // DEBUG("[scan] uuid:%u\n", (unsigned)uuid);
        if ((uuid == AD_UUID) && (sd.len == sizeof(nimble_meshconn_ad_t))) {
            /* process scan data */
            nimble_meshconn_ad_t *mcad = (nimble_meshconn_ad_t *)sd.data;
            uint32_t netid = byteorder_lebuftohl(mcad->netid);
            // DEBUG("[scan] netid:%u\n", (unsigned)netid);
            if (netid == _net.id) {
                uint32_t of = byteorder_lebuftohl(mcad->of);
                // DEBUG("[scan] of:%u\n", (unsigned)of);
                nimble_meshconn_nt_discovered(addr, rssi, mcad->of_type, of);
            }
        }
    }
}

/* context: nimble thread only */
static void _on_netif_evt(int handle, nimble_netif_event_t event,
                          const uint8_t *addr)
{
    switch (event) {
        case NIMBLE_NETIF_ACCEPTING:
            break;
        case NIMBLE_NETIF_ACCEPT_STOP:
            printf("[BLE] accept stop\n");
            _on_reconn_timeout(handle, addr);
            break;
        case NIMBLE_NETIF_INIT_MASTER:
            printf("[BLE] init master\n");
            break;
        case NIMBLE_NETIF_INIT_SLAVE:
            printf("[BLE] init slave\n");
            break;
        case NIMBLE_NETIF_CONNECTED_MASTER:
            printf("[BLE] conn master\n");
            _on_connected_up(handle, addr);
            break;
        case NIMBLE_NETIF_CONNECTED_SLAVE:
            printf("[BLE] conn slave\n");
            _on_connected_down(handle, addr);
            break;
        case NIMBLE_NETIF_CLOSED_MASTER:
            printf("[BLE] closed master\n");
            _on_closed_up(handle, addr);
            break;
        case NIMBLE_NETIF_CLOSED_SLAVE:
            printf("[BLE] closed slave\n");
            _on_closed_down(handle, addr);
            break;
        case NIMBLE_NETIF_ABORT_MASTER:
            printf("[BLE] abort master\n");
            _on_abort_up(handle, addr);
            break;
        case NIMBLE_NETIF_ABORT_SLAVE:
            printf("[BLE] sabort slave\n");
            _on_abort_down();
            break;
        case NIMBLE_NETIF_CONN_UPDATED:
            break;
        default:
            /* this should never happen */
            assert(0);
    }
}

/* context: from NimBLE thread only? */
static void _on_of_update(uint8_t of_type, uint32_t of)
{
    DEBUG("[mc] OF update - of_type:%u of:%u\n", (unsigned)of_type, (unsigned)of);
    _net.of_type = of_type;
    _net.of = of;
    _do_accept();

}

/* context: any - CALLED ONLY ONCE */
int nimble_meshconn_init(void)
{
    _net.state = STATE_IDLE;

    /* apply default parameters */
    nimble_meshconn_params(NULL);

    /* register our event callback */
    nimble_netif_eventcb(_on_netif_evt);
        _of_sub.cb = _on_of_update;
    nimble_meshconn_of_subscribe(&_of_sub);

    /* initialize timers */
    ble_npl_callout_init(&_connect_event, nimble_port_get_dflt_eventq(),
                         _do_connect, NULL);
    return 0;
}

int nimble_meshconn_params(const nimble_meshconn_params_t *params)
{
    if (params == NULL) {
        /* apply default parameters */
        _net.params.notconn_scan_itvl = NIMBLE_MESHCONN_NOTCONN_SCAN_ITVL_MS;
        _net.params.notconn_scan_win = NIMBLE_MESHCONN_NOTCONN_SCAN_WIN_MS;
        _net.params.notconn_timeout = NIMBLE_MESHCONN_NOTCONN_TIMEOUT;

        _net.params.reconn_scan_itvl = NIMBLE_MESHCONN_RECONN_SCAN_ITVL_MS;
        _net.params.reconn_scan_win = NIMBLE_MESHCONN_RECONN_SCAN_WIN_MS;
        _net.params.reconn_adv_itvl = NIMBLE_MESHCONN_RECONN_ADV_ITVL_MS;
        _net.params.reconn_timeout = NIMBLE_MESHCONN_RECONN_TIMEOUT;

        _net.params.grounded_scan_itvl = NIMBLE_MESHCONN_GROUNDED_SCAN_ITVL_MS;
        _net.params.grounded_scan_win = NIMBLE_MESHCONN_GROUNDED_SCAN_WIN_MS;
        _net.params.grounded_adv_itvl = NIMBLE_MESHCONN_GROUNDED_ADV_ITVL_MS;

        _net.params.conn_itvl_min = NIMBLE_MESHCONN_CONN_ITVL_MIN_MS;
        _net.params.conn_itvl_max = NIMBLE_MESHCONN_CONN_ITVL_MAX_MS;
        _net.params.conn_slave_latency = NIMBLE_MESHCONN_CONN_SLAVE_LATENCY;
        _net.params.conn_supervision_to = NIMBLE_MESHCONN_CONN_SUPERVISION_TO_MS;
        _net.params.conn_timeout = NIMBLE_MEHSCONN_CONN_TIMEOUT_MS;

        _net.params.links_up_min = NIMBLE_MESHCONN_LINKS_UP_MIN;
        _net.params.links_up_max = NIMBLE_MESHCONN_LINKS_UP_MAX;

    } else {
        memcpy(&_net.params, params, sizeof(_net.params));
    }

    return 0;
}

int nimble_meshconn_join(uint32_t netid)
{
    if (_net.state != STATE_IDLE) {
        return -EALREADY;
    }

    /* set state */
    _net.id = netid;
    _net.state = STATE_NOTCONN;
    _net.role = ROLE_ROUTER;    // TODO Rouer vs Node


    /* start scanning */
    // TODO: abstract to enable extended scanning...
    // nimble_meshconn_nt_set_of_type(0);
    _do_search();

    return 0;
}

int nimble_meshconn_create(uint32_t netid)
{
    if (_net.state != STATE_IDLE) {
        return -EALREADY;
    }

    DEBUG("[mc] create %u\n", (unsigned)netid);

    /* set state */
    _net.id = netid;
    _net.state = STATE_GROUNDED;
    _net.role = ROLE_BR;

    // TODO: make type configurable
    // nimble_meshconn_nt_set_of_type(NIMBLE_MESHCONN_OF_HOPCNT);
    // nimble_meshconn_of_activate(NIMBLE_MESHCONN_OF_HOPCNT, 0);    // TODO: hand over role as 2. paramter

    // nimble_meshconn_nt_set_of_type(NIMBLE_MESHCONN_OF_L2CAPPING);
    nimble_meshconn_of_activate(NIMBLE_MESHCONN_OF_L2CAPPING, 0);    // TODO: hand over role as 2. paramter

    return 0;
}

int nimble_meshconn_leave(void)
{
    if (_net.state == STATE_IDLE) {
        return -ENOTCONN;
    }

    /* reset state */
    _net.state = STATE_IDLE;

    /* close all BLE connections */
    nimble_scanner_stop();
    nimble_netif_accept_stop();
    ble_npl_callout_stop(&_connect_event);
    nimble_meshconn_nt_purge();
    int handle = nimble_netif_conn_get_next(NIMBLE_NETIF_CONN_INVALID,
                                            NIMBLE_NETIF_L2CAP_CONNECTED);
    while (handle != NIMBLE_NETIF_CONN_INVALID) {
        nimble_netif_close(handle);
        handle = nimble_netif_conn_get_next(handle, NIMBLE_NETIF_L2CAP_CONNECTED);
    }

    return 0;
}
