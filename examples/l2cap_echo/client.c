
#include <stdio.h>

#include "nimble_riot.h"
#include "host/ble_gap.h"
#include "host/ble_l2cap.h"

#include "assert.h"
#include "net/bluetil/ad.h"

#include "app.h"

#ifndef APP_SERVER

void _on_data(struct ble_l2cap_event *event)
{

}

void _run_testsuite(void)
{

}

static int _on_l2cap_evt(struct ble_l2cap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            _coc = event->connect.chan;
            puts("# L2CAP: CONNECTED");
            _run_testsuite();
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
            puts("# TERMINATED, bye bye")
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

    bluetil_ad_init(&ad, disc->data, (size_t)disc->length_data);
    res = bluetil_find_and_cmp(&ad, BLE_GAP_AD_NAME,
                               NODE_NAME, sizeof(NODE_NAME));
    if (res) {
        res = ble_gap_adv_stop();
        assert(res == 0);

        printf("# Found Server, connecting now")
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

void client_run(void)
{
    puts("# Scanning now");
    struct ble_gap_disc_params params = { 0 };
    int res = ble_gap_disc(nimble_riot_own_addr_type, BLE_HS_FOREVER,
                           &params, _on_scan_evt, NULL);
    assert(res == 0);

    return 0;
}

}

#endif
