
#include <stdio.h>

#include "nimble_riot.h"
#include "host/ble_gap.h"
#include "host/ble_l2cap.h"

#include "assert.h"
#include "thread_flags.h"
#include "net/bluetil/ad.h"

#include "app.h"



#ifndef APP_SERVER

#define FLAG_UP         (1u << 0)
#define FLAG_SYNC       (1u << 1)

/* message synchronization state */
static thread_t *_main;
// static xtimer_t _tim;

/* connection details */
static uint16_t _handle = 0;
static struct ble_l2cap_chan *_coc = NULL;
static uint32_t _seq_ack = 99;

/* data buffers */
static os_membuf_t _coc_mem[OS_MEMPOOL_SIZE(MBUFCNT, MBUFSIZE)];
static struct os_mempool _coc_mempool;
static struct os_mbuf_pool _coc_mbuf_pool;
static uint32_t _txbuf[TEST_MTU / 4];
static uint32_t _rxbuf[TEST_MTU / 4];

void _on_data(struct ble_l2cap_event *event)
{
    int res;
    (void)res;
    struct os_mbuf *rxd = event->receive.sdu_rx;
    assert(rxd != NULL);
    int rx_len = (int)OS_MBUF_PKTLEN(rxd);
    assert(rx_len <= (int)TEST_MTU);

    res = os_mbuf_copydata(rxd, 0, rx_len, _rxbuf);
    assert(res == 0);
    _seq_ack = _rxbuf[0];
    thread_flags_set(_main, FLAG_SYNC);

    /* free buffer */
    res = os_mbuf_free_chain(rxd);
    assert(res == 0);
    rxd = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
    assert(rxd != NULL);
    res = ble_l2cap_recv_ready(_coc, rxd);
    assert(res == 0);
}

static int _on_l2cap_evt(struct ble_l2cap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            _coc = event->connect.chan;
            puts("# L2CAP: CONNECTED");
            thread_flags_set(_main, FLAG_UP);
            break;
        case BLE_L2CAP_EVENT_COC_DISCONNECTED:
            _coc = NULL;
            puts("# L2CAP: DISCONNECTED");
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

static int _on_gap_evt(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    printf("# GAP event: %i\n", (int)event->type);

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT: {
            _handle = event->connect.conn_handle;
            struct os_mbuf *sdu_rx = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
            assert(sdu_rx != NULL);
            int res = ble_l2cap_connect(_handle, COC_CID, TEST_MTU, sdu_rx,
                                       _on_l2cap_evt, NULL);
            assert(res == 0);
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT:
            _handle = 0;
            puts("# TERMINATED, bye bye");
            break;
        case BLE_GAP_EVENT_CONN_UPDATE:
        case BLE_GAP_EVENT_CONN_UPDATE_REQ:
        default:
            break;
    }

    return 0;
}

static void _filter_and_connect(struct ble_gap_disc_desc *disc)
{
    int res;
    bluetil_ad_t ad;

    bluetil_ad_init(&ad, disc->data,
                    (size_t)disc->length_data, (size_t)disc->length_data);
    printf("# test one, sizeof name is %u\n", sizeof(NODE_NAME));
    res = bluetil_ad_find_and_cmp(&ad, BLE_GAP_AD_NAME,
                                  NODE_NAME, (sizeof(NODE_NAME) - 1));
    if (res) {
        res = ble_gap_disc_cancel();
        assert(res == 0);

        printf("# Found Server, connecting now");
        res = ble_gap_connect(nimble_riot_own_addr_type, &disc->addr,
                              BLE_HS_FOREVER, NULL, _on_gap_evt, NULL);
        assert(res == 0);
    }
}

static int _on_scan_evt(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_DISC:
            _filter_and_connect(&event->disc);
            break;
        case BLE_GAP_EVENT_DISC_COMPLETE:
        default:
            break;
    }

    return 0;
}

static void _send(size_t len, uint32_t seq)
{
    int res = 0;
    (void)res;
    struct os_mbuf *txd;

    assert(_coc);

    printf("# Send: size %5u seq %5u\n", len, (unsigned)seq);

    _txbuf[0] = seq;
    txd = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
    assert(txd != NULL);
    res = os_mbuf_append(txd, _txbuf, len);
    assert(res == 0);

    do {
        res = ble_l2cap_send(_coc, txd);
    } while (res == BLE_HS_EBUSY);

    if (res != 0) {
        puts("# ERROR: failed to send");
        assert(res == 0);
    }
}

void client_run(void)
{
    int res;
    (void)res;
    puts("L2CAP Echo Client");

    /* save context of the main thread */
    _main = (thread_t *)thread_get(thread_getpid());

    /* initialize of BLE related buffers */
    res = os_mempool_init(&_coc_mempool, MBUFCNT, MBUFSIZE, _coc_mem, "appbuf");
    assert(res == 0);
    res = os_mbuf_pool_init(&_coc_mbuf_pool, &_coc_mempool, MBUFSIZE, MBUFCNT);
    assert(res == 0);

    /* start scanning for a suitable test server */
    puts("# Scanning now");
    struct ble_gap_disc_params params = { 0 };
    res = ble_gap_disc(nimble_riot_own_addr_type, BLE_HS_FOREVER,
                       &params, _on_scan_evt, NULL);
    assert(res == 0);

    /* wait until we are connected to the test server */
    thread_flags_wait_all(FLAG_UP);
    puts("# Connection established, running test suite now");

    /* start running the tests */
    uint32_t seq = 0;
    memset(_txbuf, 0, sizeof(_txbuf));

    for (size_t len = 4; len <= TEST_MTU; len++) {
        /* send each size packet a number of times */
        for (unsigned repeat = 0; repeat < TEST_REPEAT; repeat++) {
            _send(len, ++seq);
            thread_flags_wait_all(FLAG_SYNC);
            if (_seq_ack != seq) {
                printf("# ERROR: unexpected sequence number received (%u)\n",
                       (unsigned)_seq_ack);
                assert(_seq_ack == seq);
            }
        }
    }

    puts("# TEST COMPLETE");

}

#endif
