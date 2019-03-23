/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Client to test and benchmark GNRC over NimBLE
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "assert.h"
#include "nimble_riot.h"
#include "nimble_netif.h"
#include "net/bluetil/ad.h"

#include "app_test_client.h"
#include "nimble_gnrc_test_conf.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

#define FLAG_UP         (1u << 0)

static thread_t *_main;
static nimble_netif_conn_t _conn;

static void _on_netif_evt(nimble_netif_conn_t *conn, nimble_netif_event_t event)
{
    assert(conn == &_conn);

    switch (event) {
        case NIMBLE_NETIF_CONNECTED:
            thread_flags_set(_main, FLAG_UP);
            break;
        case NIMBLE_NETIF_DISCONNECTED:
            puts("err: connection lost");
            assert(0);
            break;
        case NIMBLE_NETIF_ABORT:
        case NIMBLE_NETIF_CONN_UPDATED:
        default:
            break;
    }
}

static void _filter_and_connect(struct ble_gap_disc_desc *disc)
{
    int res;
    bluetil_ad_t ad;

    bluetil_ad_init(&ad, disc->data,
                    (size_t)disc->length_data, (size_t)disc->length_data);
    res = bluetil_ad_find_and_cmp(&ad, BLE_GAP_AD_NAME,
                                  APP_NODENAME, (sizeof(APP_NODENAME) - 1));
    if (res) {
        res = ble_gap_disc_cancel();
        assert(res == 0);

        printf("# Found Server, connecting now");
        res = nimble_netif_connect(&_conn, &disc->addr, NULL, BLE_HS_FOREVER);
        assert(res == NIMBLE_NETIF_OK);
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

int main(void)
{
    int res;
    (void)res;
    puts("NimBLE L2CAP test application");

    /* intialize local state */
    memset(&_conn, 0, sizeof(nimble_netif_conn_t));
    _main = (thread_t *)thread_get(thread_getpid());
    res = nimble_netif_eventcb(_on_netif_evt);
    assert(res == NIMBLE_NETIF_OK);

    /* start scanning for a suitable test server */
    puts("# Scanning now");
    struct ble_gap_disc_params params = { 0 };
    res = ble_gap_disc(nimble_riot_own_addr_type, BLE_HS_FOREVER,
                       &params, _on_scan_evt, NULL);
    assert(res == 0);

    /* wait until we are connected to the test server */
    thread_flags_wait_all(FLAG_UP);
    puts("# Connection established");

    /* extract address information? */
    app_test_client_run(&_conn.addr);

    /* should be never reached */
    return 0;
}
