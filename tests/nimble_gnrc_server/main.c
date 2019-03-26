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
 * @brief       Server to test and benchmark GNRC over NimBLE
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "assert.h"
#include "nimble_netif.h"
#include "net/bluetil/ad.h"

#include "nimble_gnrc_test_conf.h"
#include "app_udp_server.h"

/* safe AD fields */
static nimble_netif_conn_t _conn;
static uint8_t _ad_buf[BLE_HS_ADV_MAX_SZ];
static bluetil_ad_t _ad;
static struct ble_gap_adv_params _adv_params = { 0 };

static void _advertise_now(void)
{
    int res = nimble_netif_accept(&_conn, _ad.buf, _ad.pos, &_adv_params);
    (void)res;
    assert(res == NIMBLE_NETIF_OK);
    puts("# now advertising");
}

static void _on_evt(nimble_netif_conn_t *conn, nimble_netif_event_t event)
{
    assert(conn == &_conn);

    switch (event) {
        case NIMBLE_NETIF_CONNECTED:
            puts("# NIMBLE_NETIF: CONNECTED");
            break;
        case NIMBLE_NETIF_DISCONNECTED:
            puts("# NIMBLE_NETIF: DISCONNECTED");
            _advertise_now();
            break;
        case NIMBLE_NETIF_ABORT:
            break;
        case NIMBLE_NETIF_CONN_UPDATED:
            break;
        default:
            assert(0);
            break;
    }
}

int main(void)
{
    int res;
    (void)res;
    puts("NimBLE GNRC test server");

    /* start UDP server */
    app_udp_server_run();

    /* setup the NimBLE netif submodule */
    memset(&_conn, 0, sizeof(nimble_netif_conn_t));
    nimble_netif_eventcb(_on_evt);

    /* initialize advertising data and parameters */
    _adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    _adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    bluetil_ad_init_with_flags(&_ad, _ad_buf, sizeof(_ad_buf),
                               BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&_ad, APP_NODENAME);
    uint16_t ipss = BLE_SVC_IPSS;
    bluetil_ad_add(&_ad, BLE_GAP_AD_UUID16_COMP, &ipss, sizeof(ipss));

    /* start advertising the test server */
    _advertise_now();

    /* nothing else to be done in the main thread */
    return 0;
}
