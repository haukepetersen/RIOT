

#include <stdio.h>

#include "nimble_riot.h"
#include "host/ble_gap.h"
#include "host/ble_l2cap.h"

#include "assert.h"
#include "net/bluetil/ad.h"

#include "app.h"

#ifdef APP_SERVER

/* buffer allocation */
static os_membuf_t _coc_mem[OS_MEMPOOL_SIZE(MBUFCNT, MBUFSIZE)];
static struct os_mempool _coc_mempool;
static struct os_mbuf_pool _coc_mbuf_pool;

/* safe AD fields */
static uint8_t _ad_buf[BLE_HS_ADV_MAX_SZ];
static bluetil_ad_t _ad;
static struct ble_gap_adv_params _adv_params = { 0 };

/* connection details */
static uint16_t _handle = 0;
static struct ble_l2cap_chan *_coc = NULL;

static void _advertise_now(void);

static void _on_data(struct ble_l2cap_event *event)
{
    int res;
    struct os_mbuf *rxd;

    /* allocate new mbuf for receiving new data */
    rxd = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
    assert(rxd != NULL);
    res = ble_l2cap_recv_ready(_coc, rxd);
    assert(res == 0);

    rxd = event->receive.sdu_rx;
    assert(rxd != NULL);
    int rx_len =  (int)OS_MBUF_PKTLEN(rxd);
    printf("# Received %i bytes\n", rx_len);
    assert(rx_len <= (int)TEST_MTU);

    /* take received data and send it back */
    res = ble_l2cap_send(_coc, rxd);
    assert(res == 0);   /* this might (will) crash at some point ?! */
}

static int _on_gap_evt(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    printf("# GAP event %i\n", (int)event->type);

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            _handle = event->connect.conn_handle;
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            _coc = NULL;
            _handle = 0;
            _advertise_now();
            break;
        case BLE_GAP_EVENT_CONN_UPDATE:
        case BLE_GAP_EVENT_CONN_UPDATE_REQ:
        default:
            break;
    }
    return 0;
}

static int _on_l2cap_evt(struct ble_l2cap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            _coc = event->connect.chan;
            puts("# L2CAP: CONNECTED");
            break;
        case BLE_L2CAP_EVENT_COC_DISCONNECTED:
            _coc = NULL;
            puts("# L2CAP: DISCONNECTED");
            break;
        case BLE_L2CAP_EVENT_COC_ACCEPT: {
            struct os_mbuf *sdu_rx = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
            assert(sdu_rx != NULL);
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

static void _advertise_now(void)
{
    int res = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                                &_adv_params, _on_gap_evt, NULL);
    assert(res == 0);
}

void server_run(void)
{
    int res;
    (void)res;
    puts("L2CAP Echo Server");

    /* initialize of BLE related buffers */
    res = os_mempool_init(&_coc_mempool, MBUFCNT, MBUFSIZE, _coc_mem, "appbuf");
    assert(res == 0);
    res = os_mbuf_pool_init(&_coc_mbuf_pool, &_coc_mempool, MBUFSIZE, MBUFCNT);
    assert(res == 0);

    /* create l2cap server */
    res = ble_l2cap_create_server(COC_CID, TEST_MTU, _on_l2cap_evt, NULL);
    assert(res == 0);

    /* initialize advertising data and parameters */
    _adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    _adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    bluetil_ad_init_with_flags(&_ad, _ad_buf, sizeof(_ad_buf),
                               BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&_ad, NODE_NAME);
    res = ble_gap_adv_set_data(_ad.buf, (int)_ad.pos);
    assert(res == 0);

    /* start advertising this node */
    _advertise_now();

    puts("# now advertising");
}

#else
typedef int dont_be_pedantic;
#endif
