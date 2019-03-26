/*
 * Copyright (C) 2019 Freie Universität Berlin
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

#include "clist.h"
#include "shell.h"
#include "nimble_netif .h"
#include "net/bluetil/ad.h"

#include "scanlist.h"

/* number of open connections that we want to allow in parallel */
// TODO: move...
#define CONN_NUMOF          (MYNEWT_VAL_BLE_MAX_CONNECTIONS)


/* time to wait when connecting before timing out (in [ms])*/
#define MAX_WAIT            (1000U)

/* default SCAN interval */
#define SCAN_WAIT           (500U)      /* 500ms */

#define NODE_NAME           "Rudolf"

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];


/* allocate memory for connection contexts */
// TODO: move into connection manager module */
static nimble_gnrc_conn_t _conn_pool[CONN_NUMOF];
static clist_node_t _conn;

// TODO: mode
static void _conn_init(void)
{
    for (unsigned i = 0; i < CONN_NUMOF; i++) {
        memset(&_conn_pool[i], 0, sizeof(nimble_gnrc_conn_t));
        clist_rpush(&_conn, &_conn_pool[i].node);
    }
}

static void _on_blue_evt(nimble_gnrc_conn_t *conn, nimble_gnrc_event_t event)
{
    switch (event) {
        case NIMBLE_GNRC_CONNECTED:
            printf("BLE EVT: conn %p is now CONNECTED\n", (void *)conn);
            break;
        case NIMBLE_GNRC_DISCONNECTED:
            printf("BLE EVT: conn %p is now DISCONNECTED\n", (void *)conn);
            clist_rpush(&_conn, &conn->node);
            break;
        case NIMBLE_GNRC_ABORT:
            printf("BLE EVT: conn %p -> ABORT\n", (void *)conn);
            clist_rpush(&_conn, &conn->node);
            break;
        case NIMBLE_GNRC_CONN_UPDATED:
            printf("BLE EVT: conn %p was UPDATED\n", (void *)conn);
            break;
        default:
            puts("error: unknown connection event\n");
            break;
    }
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

static void _dump_addr(const uint8_t *addr)
{
    for (int i = 0; i < 5; i++) {
        printf("%x:", (int)addr[i]);
    }
    printf("%x\n", addr[5]);
}

static int _cmd_ble_info(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (unsigned i = 0; i < CONN_NUMOF; i++) {
        printf("[%2u] ", i);
        if (nimble_gnrc_is_connected(&_conn_pool[i])) {
            printf("CONNECTED to ");
            _dump_addr(_conn_pool[i].addr);
        }
        else if (nimble_gnrc_is_advertising(&_conn_pool[i])) {
            printf("ADVERTISING\n");
        }
        else {
            printf("unused\n");
        }
    }

    return 0;
}

static int _cmd_ble_scan(int argc, char **argv)
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

static int _cmd_ble_conn(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <scanlist entry #>\n", argv[0]);
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

    nimble_gnrc_conn_t *conn = (nimble_gnrc_conn_t *)clist_lpop(&_conn);
    if (conn == NULL) {
        puts("error: no connection context available\n");
        return 1;
    }

    printf("Connecting to ");
    scanlist_print_entry(e);

    // TODO: configure connection parameters somehow?
    int res = nimble_gnrc_connect(conn, &e->addr, NULL, MAX_WAIT);
    if (res != 0) {
        printf("error: unable to trigger connection procedure (%i)\n", res);
        return 1;
    }

    return 0;
}

static int _cmd_ble_adv(int argc, char **argv)
{
    // TODO: allow to specify name etc via cmd line?!
    (void)argc;
    (void)argv;

    nimble_gnrc_conn_t *conn = (nimble_gnrc_conn_t *)clist_lpop(&_conn);
    if (conn == NULL) {
        puts("error: no connection memory available\n");
        return 1;
    }

    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&ad, NODE_NAME);
    uint16_t ipss_uuid = BLE_SVC_IPSS;
    bluetil_ad_add(&ad, BLE_GAP_AD_UUID16_COMP, &ipss_uuid, sizeof(uint16_t));

    struct ble_gap_adv_params adv_params = { 0 };
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    int res = nimble_gnrc_accept(conn, ad.buf, ad.pos, &adv_params);
    if (res != 0) {
        printf("error: failed to trigger advertising (%i)\n", res);
        return 1;
    }

    puts("success: node is now being advertised\n");
    return 0;
}

static int _cmd_ble_kill(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <connection slot>\n", argv[0]);
        return 1;
    }

    unsigned slot = atoi(argv[1]);
    if (slot >= CONN_NUMOF) {
        puts("error: given slot is invalid\n");
        return 1;
    }

    int res = nimble_gnrc_terminate(&_conn_pool[slot]);
    if (res != NIMBLE_GNRC_OK) {
        printf("error: unable to kill given connection [%i]\n", (int)res);
        return 1;
    }
    return 0;
}

static const shell_command_t _cmds[] = {
    { "ble_info", "show list of open BLE connections", _cmd_ble_info },
    { "ble_scan", "discover neighboring nodes", _cmd_ble_scan },
    { "ble_conn", "connect to a selected node", _cmd_ble_conn },
    { "ble_adv", "start advertising this node", _cmd_ble_adv },
    { "ble_kill", "terminate BLE given BLE connection", _cmd_ble_kill },
    { NULL, NULL, NULL }
};

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("Example application for 6LoWPAN over BLE using NimBLE and GNRC");


    /* create a list for keeping scan results */
    scanlist_init();

    /* initialize connection list */
    _conn_init();
    nimble_gnrc_eventcb(_on_blue_evt);

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(_cmds, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
