/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example for showing 6LoWPAN over BLE using NimBLE and GNRC
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>

#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/util/util.h"

#include "clist.h"
#include "shell.h"
#include "net/bluetil/ad.h"

#include "scanlist.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

/* local configuration */
#define NODE_NAME           "l2cap_bench"
#define CONN_WAIT           (1000U)
#define SCAN_WAIT           (500U)      /* 500ms */

#define COC_CID             (0x0235)    /* random value */

/* MTU and buffer configuration */
#define MTUSIZE             (5000)      /* up to 5K packets used in this app */
#define MTUBUF_SPLIT        (50)        /* split in 100 byte buffers */
#define MTUBUF_NUM          (3)         /* buffer up to 3 full MTUs */

#define MBUFSIZE_OVHD       (sizeof(struct os_mbuf) + \
                             sizeof(struct os_mbuf_pkthdr))
#define MBUFSIZE_PAYLOAD    (MTUSIZE / MTUBUF_SPLIT)
#define MBUFSIZE            (MBUFSIZE_PAYLOAD + MBUFSIZE_OVHD)
#define MBUFCNT             (MTUBUF_NUM * MTUBUF_SPLIT)

/* event flags */
#define EVT_SINGLE          (0x66)
#define EVT_START           (0x99)
#define EVT_END             (0x33)

static uint16_t _handle = 0;
static struct ble_l2cap_chan *_coc = NULL;
static uint8_t _own_addr_type;

/* buffer allocation */
static os_membuf_t _coc_mem[OS_MEMPOOL_SIZE(MBUFCNT, MBUFSIZE)];
static struct os_mempool _coc_mempool;
static struct os_mbuf_pool _coc_mbuf_pool;

static uint8_t _txbuf[MTUSIZE];
static uint8_t _rxbuf[MTUSIZE];

static uint8_t _rx_last;
static uint32_t _rx_time;
static unsigned _rx_count;

static int _send(void *data, size_t len, unsigned num)
{
    struct os_mbuf *sdu_tx = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
    if (!sdu_tx) {
        puts("[nim] send: error - unable to allocate mbuf");
        return 1;
    }

    int res = os_mbuf_append(sdu_tx, data, len);
    if (res != 0) {
        os_mbuf_free_chain(sdu_tx);
        printf("[nim] send: error - unable to append data (%i)\n", res);
        return res;
    }

    do {
        res = ble_l2cap_send(_coc, sdu_tx);
    } while (res == BLE_HS_EBUSY);

    if (res != 0) {
        os_mbuf_free_chain(sdu_tx);
        printf("[nim] send: error - unable to send SDU (%i)\n", res);
    }
    else {
        printf("[nim] send: OK - #%u\n", num);
    }

    return res;
}

static void _on_data(struct ble_l2cap_event *event)
{
    (void)event;

    struct os_mbuf *rxd = event->receive.sdu_rx;
    int rx_len =  (int)OS_MBUF_PKTLEN(rxd);
    int res = os_mbuf_copydata(rxd, 0, rx_len, _rxbuf);
    if (res != 0) {
        printf("[nim] RX: error copying received data (%i)\n", res);
    }
    res = os_mbuf_free_chain(rxd);
    if (res != 0) {
        printf("[nim] RX: error freeing rxd\n");
    }

    _rx_count += (unsigned)rx_len;
    if (_rxbuf[1] == EVT_START) {
        _rx_last = (_rxbuf[0] - 1);
        _rx_time = xtimer_now_usec();
        _rx_count = (unsigned)rx_len;
    }
    else if (_rxbuf[1] == EVT_END) {
        _rx_time = xtimer_now_usec() - _rx_time;
        printf("Received %u bytes in %uus (%u byte/s)\n",
               _rx_count, (unsigned)_rx_time,
               (unsigned)((_rx_count * 1000) / (_rx_time / 1000)));
    }
    else if (_rxbuf[1] == EVT_SINGLE) {
        printf("Received %u bytes\n", (unsigned)rx_len);
    }
    else {
        if (_rx_last != (_rxbuf[0] - 1)) {
            printf("LOSS: #%i was lost\n", (int)(_rxbuf[0] - 1));
        }
    }

    _rx_last = _rxbuf[0];

    /* allocate new buffer */
    rxd = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
    if (rxd == NULL) {
        puts("[nim] on data: ERRRO - unable to allocate next SDU");
        assert(0);
    }
    ble_l2cap_recv_ready(_coc, rxd);
}

static int _on_l2cap_client_evt(struct ble_l2cap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            _coc = event->connect.chan;
            puts("[nim] l2cap: CONNECTED");
            break;
        case BLE_L2CAP_EVENT_COC_DISCONNECTED:
            _coc = NULL;
            puts("[nim] l2cap: DISCONNECTED");
            break;
        case BLE_L2CAP_EVENT_COC_DATA_RECEIVED:
            _on_data(event);
            break;
        case BLE_L2CAP_EVENT_COC_ACCEPT:
            /* this event should never be triggered for the L2CAP client */
            /* fallthrough */
        default:
            assert(0);
            break;
    }

    return 0;
}

static int _on_l2cap_server_evt(struct ble_l2cap_event *event, void *arg)
{
    (void)arg;
    // DEBUG("[nim] l2cap server: event %i\n", (int)event->type);

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            _coc = event->connect.chan;
            puts("[nim] l2cap: CONNECTED");
            break;
        case BLE_L2CAP_EVENT_COC_DISCONNECTED:
            _coc = NULL;
            puts("[nim] l2cap: DISCONNECTED");
            break;
        case BLE_L2CAP_EVENT_COC_ACCEPT: {
            struct os_mbuf *sdu_rx = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
            if (sdu_rx == NULL) {
                DEBUG("[nim] l2cap ACCEPT: error: no buffer\n");
                return 1;
            }
            ble_l2cap_recv_ready(event->accept.chan, sdu_rx);
            break;
        }
        case BLE_L2CAP_EVENT_COC_DATA_RECEIVED:
            _on_data(event);
            break;
        default:
            assert(0);
            break;
    }

    return 0;
}

static int _on_gap_master_evt(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    DEBUG("[nim] GAP master: event %i\n", (int)event->type);

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT: {
            _handle = event->connect.conn_handle;
            struct os_mbuf *sdu_rx = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
            int res = ble_l2cap_connect(_handle, COC_CID, MTUSIZE, sdu_rx,
                                       _on_l2cap_client_evt, NULL);
            DEBUG("[nim] GAP master: l2cap connect %i\n", res);
            assert(res == 0);
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT:
            _coc = NULL;
            _handle = 0;
            break;
        case BLE_GAP_EVENT_CONN_UPDATE:
            DEBUG("[nim] gap: CONN_UPDATE\n");
            break;
        case BLE_GAP_EVENT_CONN_UPDATE_REQ:
            DEBUG("[nim] gap: CONN_UPDATE_REQ\n");
            break;
        default:
            assert(0);
            break;
    }

    return 0;
}

static int _on_gap_slave_evt(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    DEBUG("[nim] GAP slave: event %i\n", (int)event->type);

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            _handle = event->connect.conn_handle;
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            _coc = NULL;
            _handle = 0;
            break;
        case BLE_GAP_EVENT_CONN_UPDATE:
            DEBUG("[nim] gap: CONN_UPDATE\n");
            break;
        case BLE_GAP_EVENT_CONN_UPDATE_REQ:
            DEBUG("[nim] gap: CONN_UPDATE_REQ\n");
            break;
        default:
            assert(0);
            break;
    }
    return 0;
}


static int _on_scan_evt(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_DISC:
            scanlist_update(&event->disc.addr, event->disc.data,
                            event->disc.length_data);
            break;
        case BLE_GAP_EVENT_DISC_COMPLETE:
            puts("Scan complete, results:\n");
            scanlist_print();
            break;
        default:
            break;
    }

    return 0;
}

static int _cmd_scan(int argc, char **argv)
{
    uint32_t to = SCAN_WAIT;
    if (argc >= 2) {
        to = (uint32_t)atoi(argv[1]);
    }

    if (ble_gap_disc_active()) {
        ble_gap_disc_cancel();
    }

    scanlist_clear();

    struct ble_gap_disc_params params = { 0 };
    uint8_t atype;
    ble_hs_id_infer_auto(0, &atype);
    int res = ble_gap_disc(atype, to, &params, _on_scan_evt, NULL);
    if (res != 0) {
        printf("error: unable to start scanning (%i)\n", res);
        return 1;
    }

    return 0;
}

static int _cmd_connect(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <scanlist entry #>\n", argv[0]);
        return 1;
    }

    if (_coc != NULL) {
        puts("error: already connected");
        return 1;
    }
    if (ble_gap_disc_active()) {
        puts("error: scan is in progress");
        return 1;
    }
    if (ble_gap_conn_active()) {
        puts("error: connection procedure in progress");
        return 1;
    }

    scanlist_t *e = scanlist_get((unsigned)atoi(argv[1]));
    if (e == NULL) {
        puts("error: invalid scanlist entry # given");
        return 1;
    }
    printf("Connecting to ");
    scanlist_print_entry(e);

    // TODO: connection parameters
    int res = ble_gap_connect(_own_addr_type, &e->addr, (int32_t)CONN_WAIT,
                              NULL, _on_gap_master_evt, NULL);
    if (res != 0) {
        printf("error: unable to connect (%i)\n", res);
    }
    else {
        puts("connection ok\n");
    }
    return 0;
}

static int _cmd_listen(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int res;

    if (_coc != NULL) {
        puts("error: already connected");
        return 1;
    }

    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&ad, NODE_NAME);
    res = ble_gap_adv_set_data(ad.buf, (int)ad.pos);
    if (res != 0) {
        printf("error: unable to set advertising data (%i)\n", res);
        return 1;
    }

    struct ble_gap_adv_params adv_params = { 0 };
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    res = ble_gap_adv_start(_own_addr_type, NULL, BLE_HS_FOREVER,
                                &adv_params, _on_gap_slave_evt, NULL);
    if (res != 0) {
        printf("error: failed to trigger advertising (%i)\n", res);
        return 1;
    }

    puts("success: node is now being advertised\n");
    return 0;
}

static int _cmd_send(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <num of bytes>\n", argv[0]);
        return 1;
    }

    if (_coc == NULL) {
        puts("error: not connected\n");
        return 1;
    }

    size_t cnt = (size_t)atoi(argv[1]);
    if (cnt > MTUSIZE) {
        puts("error: data size exceeds MTU size");
        return 1;
    }

    printf("[nim] send: pkt #%03i -> %u byte\n", (int)_txbuf[0], cnt);

    ++_txbuf[0];
    _txbuf[1] = EVT_SINGLE;

    int res = _send(_txbuf, cnt, (int)_txbuf[0]);
    if (res != 0) {
        printf("error: unable to send data (%i)\n", res);
        return 1;
    }

    return 0;
}

static int _cmd_stats(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (_handle != 0) {
        puts("GAP: connected");
    }
    else {
        puts("GAP: not connected");
    }
    if (_coc != NULL) {
        puts("L2CAP: connected");
        int our_mtu = ble_l2cap_get_our_mtu(_coc);
        int peer_mtu = ble_l2cap_get_peer_mtu(_coc);
        printf("       our MTU: %i, peer MTU: %i\n", our_mtu, peer_mtu);
    }
    else {
        puts("L2CAP: not connected");
    }

    return 0;
}

static int _cmd_clear(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    /* TODO */
    return 0;
}

static int _cmd_flood(int argc, char **argv)
{

    if (argc < 3) {
        printf("usage: %s <pkt size> <pkt cnt>\n", argv[0]);
        return 1;
    }

    if (_coc == NULL) {
        puts("error: not connected");
        return 1;
    }

    size_t sum = 0;
    size_t size = (size_t)atoi(argv[1]);
    unsigned cnt = (unsigned)atoi(argv[2]);

    if (size > MTUSIZE) {
        puts("error: pkt size exceeds buffer space\n");
        return 1;
    }

    uint32_t time = xtimer_now_usec();
    for (unsigned i = 0; i < cnt; i++) {
        if (i == 0) {
            _txbuf[1] = EVT_START;
        }
        else if (i == (cnt - 1)) {
            _txbuf[1] = EVT_END;
        }
        else {
            _txbuf[1] = 0;
        }
        _txbuf[0] = (uint8_t)i;

        _send(_txbuf, size, i);
        sum += size;
    }
    time = xtimer_now_usec() - time;

    printf("Send %u bytes in %u us (%u byte/s)\n", sum, (unsigned)time,
           (unsigned)((sum * 1000) / (time / 1000)));

    return 0;
}

static int _cmd_bench(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    /* TODO */
    return 0;
}

static int _cmd_connup(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <interval in us>\n", argv[0]);
        return 1;
    }

    if (_handle == 0) {
        puts("error: not connected\n");
        return 1;
    }

    unsigned interval = (unsigned)atoi(argv[1]);
    uint16_t itvl = (interval / 1500);

    struct ble_gap_upd_params params;
    params.itvl_min = itvl;
    params.itvl_max = itvl;
    params.latency = 0;
    params.supervision_timeout = 100;       /* 1s */
    params.min_ce_len = 1;
    params.max_ce_len = itvl * 2;

    int res = ble_gap_update_params(_handle, &params);
    if (res == BLE_HS_EINVAL) {
        puts("error: given parameter invalid");
        return 1;
    }
    else if (res != 0) {
        printf("error: unable to update parameters (%i)\n", res);
    }

    printf("connection parameters updated, interval is now %uus\n",
           (((unsigned)itvl) * 1500));
    return 0;
}

static const shell_command_t _cmds[] = {
    { "scan", "scan for BLE neighbors", _cmd_scan },
    { "connect", "establish a connection", _cmd_connect },
    { "listen", "accept a new connection", _cmd_listen },
    { "send", "send data", _cmd_send },
    { "stats", "show stats", _cmd_stats },
    { "clear", "clear statistics", _cmd_clear },
    { "flood", "flood connection with data", _cmd_flood },
    { "bench", "measure the incoming data", _cmd_bench },
    { "connup", "connection parameter update", _cmd_connup },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("Benchmark L2CAP Throughput");

    /* create a list for keeping scan results */
    scanlist_init();

    /* initialize transmit buffer counter */
    _txbuf[0] = 0;

    /* initialize of BLE related buffers */
    if (os_mempool_init(&_coc_mempool, MBUFCNT, MBUFSIZE,
                        _coc_mem, "appbuf") != 0) {
        return 1;
    }
    if (os_mbuf_pool_init(&_coc_mbuf_pool, &_coc_mempool,
                          MBUFSIZE, MBUFCNT) != 0) {
        return 1;
    }

    int res = ble_hs_util_ensure_addr(0);
    if (res != 0) {
        return 1;
    }
    res = ble_hs_id_infer_auto(0, &_own_addr_type);
    if (res != 0) {
        return 1;
    }

    res = ble_l2cap_create_server(COC_CID, MTUSIZE,
                                  _on_l2cap_server_evt, NULL);
    if (res != 0) {
        return 1;
    }

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(_cmds, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
