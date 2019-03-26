/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble
 * @{
 *
 * @file
 * @brief       CCN-lite face adapter for NimBLE using L2CAP as transport
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "nimble_blueface.h"


#define CID_CCN         (0x2323)

#define COC_BUF_CNT     (5U)
#define COC_MAXMTU      (256U)

/* allocate shared mbuf pool */
static os_membuf_t _mem[OS_MEMPOOL_SIZE(COC_BUF_CNT, COC_MAXMTU)];
static struct os_mempool _mempool;
static struct os_mbuf_pool _mbuf_pool;

static int _is_connected(nimble_blueface_t *bf)
{
    return (bf->chan != NULL);
}

static int _ccnl_face_up(nimble_blueface_t *bf)
{
    /* TODO: call ccnl face up function */
    return 0;
}

static int _ccnl_face_down(nimble_blueface_t *bf)
{
    /* TODO: call ccnl face down function */
    return 0;
}

static int _on_l2cap_evt(struct ble_l2cap_event *event, void *arg)
{
    nimble_blueface_t *bf = (nimble_blueface_t *)arg;

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            printf("#### BLE_L2CAP_EVENT_COC_CONNECTED ####\n");
            _ccnl_face_up(bf);
            break;
        case BLE_L2CAP_EVENT_COC_DISCONNECTED:
            printf("#### BLE_L2CAP_EVENT_COC_DISCONNECTED ####\n");
            _ccnl_face_down(bf);
            break;
        case BLE_L2CAP_EVENT_COC_ACCEPT:
            printf("#### BLE_L2CAP_EVENT_COC_ACCEPT ####\n");
            break;
        case BLE_L2CAP_EVENT_COC_DATA_RECEIVED:
            printf("#### BLE_L2CAP_EVENT_COC_DATA_RECEIVED ####\n");
            break;
        default:
            printf("l2cap: server - unkown event (%i)\n", (int)event->type);
            break;
    }

    return 0;
}

static int _on_gap_evt(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            puts("\n##### BLE_GAP_EVENT_CONNECT ####");
            // uint16_t conn_handle = event->connect.conn_handle;

            struct os_mbuf *sdu_rx = os_mbuf_get_pkthdr(&_mbuf_pool, 0);
            int res = ble_l2cap_connect(event->connect.conn_handle,
                                        CID_CCN, COC_MAXMTU, sdu_rx,
                                        _on_l2cap_evt, arg);
            if (res != 0) {
                // TODO
            }

            break;
        case BLE_GAP_EVENT_DISCONNECT:
            puts("\n##### BLE_GAP_EVENT_CONNECT #####");
            break;
        default:
            break;
    }

    return 0;
}

int nimble_blueface_init(void)
{
    /* initialize memory pools used for the l2cap channel */
    int res = os_mempool_init(&_mempool, COC_BUF_CNT, COC_MAXMTU, _mem, "ccnl");
    if (res != 0) {
        return -1;
    }
    res = os_mbuf_pool_init(&_mbuf_pool, &_mempool, COC_MAXMTU, COC_BUF_CNT);
    if (res != 0) {
        return -2;
    }

    return 0;
}

int nimble_blueface_connect(nimble_blueface_t *bf,
                            ble_addr_t *addr, uint32_t max_wait,
                            nimble_blueface_evt_cb_t on_evt,
                            nimble_blueface_data_cb_t on_data,
                            void *arg)
{
    (void)on_evt;
    (void)on_data;
    (void)arg;

    if (_is_connected(bf)) {
        return -1;
    }

    bf->evt_cb = on_evt;
    bf->data_cb = on_data;

    uint8_t own_addr_type = 11; // TODO

    // TODO: make connection parameters configurable
    int res = ble_gap_connect(own_addr_type, addr, max_wait, NULL,
                              _on_gap_evt, bf);

    if (res != 0) {
        return -1;
    }

    return 0;
}

int nimble_blueface_terminate(nimble_blueface_t *bf)
{
    (void)bf;

    return -23;
}

int nimble_blueface_send(nimble_blueface_t *bf, void *data, size_t len)
{
    (void)bf;
    (void)data;
    (void)len;

    return 0;
}
