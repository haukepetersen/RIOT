/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_facemaster_faceimp
 *
 * # TODO
 * - many many things...
 *
 *
 * @{
 *
 * @file
 * @brief       NimBLE specific, BLE-based face implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "assert.h"

#include "net/facemaster.h"
#include "net/faceimp/nimble.h"

#include "host/ble_hs.h"
#include "host/util/util.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

// TODO: move to more central location (some public header) to make this
// configurable
#define CID_CCN         (0x2323)

// TODO: refine the actual used buffer size and make it configurable
#define COC_BUF_CNT     (5U)
// TODO: make target MTU size dynamically configurable?!
// TODO: join this with FACEIMP_NIMBLE_MAXMTU? Understand more of mbufs first...
#define COC_MAXMTU      (250U)

static os_membuf_t _mem[OS_MEMPOOL_SIZE(COC_BUF_CNT, COC_MAXMTU)];
static struct os_mempool _mem_pool;
static struct os_mbuf_pool _mbuf_pool;

static uint8_t _rxbuf[FACEIMP_NIMBLE_MAXMTU];

static uint8_t _own_addr_type;

static faceimp_nimble_eventcb_t _eventcb = NULL;

static const faceimp_hooks_t _nim_imp_hooks;

static int _on_data(faceimp_nimble_t *nim_imp, struct ble_l2cap_event *event)
{
    struct os_mbuf *rxb = event->receive.sdu_rx;

    size_t rx_len = (size_t)OS_MBUF_PKTLEN(rxb);
    if (rx_len > FACEIMP_NIMBLE_MAXMTU) {
        DEBUG("[faceimp_nimble] _on_data: size exceeds MTU size, dropping packet\n");
        return 1;
    }

    DEBUG("[faceimp_nimble] _on_data: received %u bytes\n", rx_len);

    /* copy the receive data and free the mbuf */
    os_mbuf_copydata(rxb, 0, rx_len, _rxbuf);
    os_mbuf_free_chain(rxb);
    /* free the mbuf and allocate a new one for receiving new data */
    rxb = os_mbuf_get_pkthdr(&_mbuf_pool, 0);
    if (rxb == NULL) {
        // TODO: better error handling...
        return 1;
    }
    ble_l2cap_recv_ready(event->receive.chan, rxb);


    /* give data to the Facemaster and subsequently the ICN implementation */
    facemaster_recv(&nim_imp->imp, _rxbuf, rx_len);

    return FACEIMP_OK;
}

static int _on_l2cap_evt(struct ble_l2cap_event *event, void *arg)
{
    int res = 0;
    faceimp_nimble_t *nim_imp = (faceimp_nimble_t *)arg;

    DEBUG("[faceimp_nimble] _on_l2cap_evt: event %i\n", (int)event->type);

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            if (nim_imp->l2cap_coc == NULL) {
                nim_imp->l2cap_coc = event->connect.chan;
            }
            // TODO: check what happens if return value is != 0...
            nim_imp->conn_handle = event->connect.conn_handle;
            res = facemaster_up(&nim_imp->imp);
            if (_eventcb) {
                _eventcb(nim_imp, FACEIMP_NIMBLE_CONNECTED);
            }
            break;
        case BLE_L2CAP_EVENT_COC_DISCONNECTED:
            /* nothing to be done here */
            break;
        case BLE_L2CAP_EVENT_COC_ACCEPT: {
            struct os_mbuf *sdu_rx = os_mbuf_get_pkthdr(&_mbuf_pool, 0);
            if (sdu_rx == NULL) {
                // terminate BLE connection through GAP?!
                nim_imp->l2cap_coc = NULL;
                res = 1;
            }
            nim_imp->l2cap_coc = event->accept.chan;
            ble_l2cap_recv_ready(nim_imp->l2cap_coc, sdu_rx);
            break;
        }
        case BLE_L2CAP_EVENT_COC_DATA_RECEIVED:
            res = _on_data(nim_imp, event);
            break;
        default:
            break;
    }

    return res;
}

static int _on_gap_terminated(faceimp_nimble_t *nim_imp)
{
    facemaster_down(&nim_imp->imp);

    nim_imp->conn_handle = 0;
    nim_imp->l2cap_coc = NULL;

    if (_eventcb) {
        _eventcb(nim_imp, FACEIMP_NIMBLE_DISCONNECTED);
    }

    return FACEIMP_OK;
}

static int _on_gap_slave_evt(struct ble_gap_event *event, void *arg)
{
    faceimp_nimble_t *nim_imp = (faceimp_nimble_t *)arg;
    int res = 0;

    DEBUG("[faceimp_nimble] _on_gap_master_evt: event %i\n", (int)event->type);

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT: {
            struct os_mbuf *sdu_rx = os_mbuf_get_pkthdr(&_mbuf_pool, 0);
            if (sdu_rx == NULL) {
                // TODO
                return 1;
            }
            res = ble_l2cap_connect(event->connect.conn_handle,
                                    CID_CCN, COC_MAXMTU, sdu_rx,
                                    _on_l2cap_evt, nim_imp);
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT:
            res = _on_gap_terminated(nim_imp);
            break;
        default:
            /* do nothing */
            break;
    }

    return res;
}

static int _on_gap_master_evt(struct ble_gap_event *event, void *arg)
{
    faceimp_nimble_t *nim_imp = (faceimp_nimble_t *)arg;
    int res = 0;

    DEBUG("[faceimp_nimble] _on_gap_slave_evt: event %i\n", (int)event->type);

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            res = ble_l2cap_create_server(CID_CCN, COC_MAXMTU,
                                          _on_l2cap_evt, nim_imp);
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            res = _on_gap_terminated(nim_imp);
            break;
        default:
            /* do nothing */
            break;
    }

    return res;
}

static int _hook_send(faceimp_t *imp, void *data, size_t len)
{
    assert(imp);
    assert(data);
    assert(len > 0);        /* for now: no support for empty packets */

    faceimp_nimble_t *nim_imp = (faceimp_nimble_t *)imp;

    if (nim_imp->l2cap_coc == NULL) {
        return FACEIMP_NOTCONN;
    }

    struct os_mbuf *sdu = os_mbuf_get_pkthdr(&_mbuf_pool, 0);
    if (!sdu) {
        DEBUG("[faceimp_nimble] _send: unable to allocate mbuf\n");
        return FACEIMP_NOMEM;
    }
    int res = os_mbuf_append(sdu, data, len);
    if (res != 0) {
        DEBUG("[faceimp_nimble] _send: unable to append data\n");
        os_mbuf_free_chain(sdu);
        return FACEIMP_NOMEM;
    }

    res = ble_l2cap_send(nim_imp->l2cap_coc, sdu);
    if (res != 0) {
        DEBUG("[faceimp_nimble] _send: error sending SDU (%i)\n", res);
        return FACEIMP_DEVERR;
    }

    return FACEIMP_OK;
}

static int _hook_terminate(faceimp_t *imp)
{
    return faceimp_nimble_terminate((faceimp_nimble_t *)imp);
}

static const faceimp_hooks_t _nim_imp_hooks = {
    .send = _hook_send,
    .terminate = _hook_terminate,
};

int faceimp_nimble_init(void)
{
    /* initialize memory and mbuf pools shared by all COC channels we use */
    if (os_mempool_init(&_mem_pool, COC_BUF_CNT, COC_MAXMTU, _mem,
                              "faceimp_nimble") != 0) {
        return FACEMASTER_NOMEM;
    }
    if (os_mbuf_pool_init(&_mbuf_pool, &_mem_pool, COC_MAXMTU, COC_BUF_CNT) != 0) {
        return FACEMASTER_NOMEM;
    }

    /* make sure synchronization of host and controller is done, this should
     * always be the case */
    while (!ble_hs_synced()) {}

    /* get our own address type for future reference (can probably be optimized
     * away at some point) */
    int res = ble_hs_util_ensure_addr(0);
    if (res != 0) {
        return FACEIMP_DEVERR;
    }
    res = ble_hs_id_infer_auto(0, &_own_addr_type);
    if (res != 0) {
        return FACEIMP_DEVERR;
    }

    return FACEIMP_OK;
}

int faceimp_nimble_eventcb(faceimp_nimble_eventcb_t cb)
{
    _eventcb = cb;
    return FACEIMP_OK;
}

int faceimp_nimble_connect(faceimp_nimble_t *nim_imp, ble_addr_t *addr,
                           const struct ble_gap_conn_params *conn_params,
                           uint32_t connect_timeout)
{
    assert(nim_imp);
    assert(addr);

    if (nim_imp->l2cap_coc != NULL) {
        DEBUG("[faceimp_nimble] connect: busy\n");
        return FACEMASTER_BUSY;
    }
    // TODO: also protect Imp against a connection process already in progress

    /* initialize the base Imp */
    // TODO: possibly move this to its own function? */
    nim_imp->imp.hooks = &_nim_imp_hooks;

    int res = ble_gap_connect(_own_addr_type, addr, connect_timeout,
                              conn_params, _on_gap_slave_evt, nim_imp);
    if (res != 0) {
        DEBUG("[faceimp_nimble] connect: gap_connect failed (%i)\n", res);
        return FACEMASTER_FACEERR;
    }

    /* connection process initiated successfully */
    return FACEIMP_OK;
}

int faceimp_nimble_update(faceimp_nimble_t *nim_imp,
                          struct ble_gap_conn_params *conn_params)
{
    assert(nim_imp);

    (void)nim_imp;
    (void)conn_params;

    return FACEIMP_NOTSUP;
}

int faceimp_nimble_accept(faceimp_nimble_t *nim_imp,
                          void *ad, size_t ad_len,
                          const struct ble_gap_adv_params *adv_params)
{
    assert(nim_imp);    // TODO: allow to set to NULL for non-connectable adv?
    assert(adv_params);

    /* terminate any advertisement that might be in progress */
    // TODO: notify user about this?
    // TODO: detect if accept() call is simply an update on adv parameters?!
    // TODO: -> NO, we abort if something is advertising. But cancel advertising
    //          using the terminate function?!
    if (ble_gap_adv_active() != 0) {
        ble_gap_adv_stop();
    }

    nim_imp->imp.hooks = &_nim_imp_hooks;

    /* configure advertising data (managed by a faceman) */
    int res = ble_gap_adv_set_data((const uint8_t *)ad, (int)ad_len);
    if (res != 0) {
        return FACEIMP_DEVERR;
    }
    /* start actually advertising */
    res = ble_gap_adv_start(_own_addr_type, NULL, BLE_HS_FOREVER,
                                adv_params, _on_gap_master_evt, nim_imp);
    // TODO: parse return value to explicitly signal if no connection context
    //       is available anymore?!
    if (res != 0) {
        return FACEIMP_DEVERR;
    }

    return FACEIMP_OK;
}

int faceimp_nimble_terminate(faceimp_nimble_t *nim_imp)
{
    assert(nim_imp);

    if (nim_imp->conn_handle == 0) {
        return FACEIMP_NOTCONN;
    }

    int res = ble_gap_terminate(nim_imp->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    if (res != 0) {
        return FACEIMP_DEVERR;
    }

    return FACEIMP_OK;
}
