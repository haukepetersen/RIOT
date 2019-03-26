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
 * @brief       CCN-lite over BLE demonstrator application
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>

#include "fmt.h"
#include "shell.h"
#include "thread.h"
#include "nimble_riot.h"
#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/util/util.h"

#include "scanlist.h"

#define CID_CCN         (0x2323)

#define COC_BUF_CNT     (5U)
#define COC_MAXMTU      (256U)

static char _stack[THREAD_STACKSIZE_DEFAULT];
static uint8_t _own_addr_type;

static uint16_t _con_handle = 0;
static struct ble_l2cap_chan *_coc_chan = NULL;

/* TMP: allocate memory for incoming and outgoing COC SDUs */
static os_membuf_t _coc_mem[OS_MEMPOOL_SIZE(COC_BUF_CNT, COC_MAXMTU)];
static struct os_mempool _coc_mempool;
static struct os_mbuf_pool _coc_mbuf_pool;


static void _coc_con(uint16_t handle);


static int _read_addr(ble_addr_t *addr, char *astr, size_t len)
{
    /* make sure it a BLE addr */
    if (len < 17) {
        return -1;
    }
    for (unsigned i = 2; i < 17; i += 3) {
        if (astr[i] != ':') {
            return -1;
        }
    }

    unsigned pos = 5;
    for (unsigned i = 0; i < 17; i += 3) {
        addr->val[pos--] = fmt_hex_byte(&astr[i]);
    }

    return 0;
}

static void _print_addr(ble_addr_t *addr)
{
    printf("%02x", (int)addr->val[5]);
    for (int i = 4; i >= 0; i--) {
        printf(":%02x", addr->val[i]);
    }
    switch (addr->type) {
        case BLE_ADDR_PUBLIC:       printf("(PUBLIC)\n");   break;
        case BLE_ADDR_RANDOM:       printf("(RANDOM)\n");   break;
        case BLE_ADDR_PUBLIC_ID:    printf("(PUB_ID)\n");   break;
        case BLE_ADDR_RANDOM_ID:    printf("(RAND_ID)\n");  break;
        default:                    printf("(UNKNOWN)\n");  break;
    }
}

static int _on_l2cap_server_event(struct ble_l2cap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            printf("#### BLE_L2CAP_EVENT_COC_CONNECTED ####\n");
            break;
        case BLE_L2CAP_EVENT_COC_DISCONNECTED:
            printf("#### BLE_L2CAP_EVENT_COC_DISCONNECTED ####\n");
            _coc_chan = NULL;

            break;
        case BLE_L2CAP_EVENT_COC_ACCEPT:
            printf("#### BLE_L2CAP_EVENT_COC_ACCEPT ####\n");
            struct os_mbuf *sdu_rx;
            sdu_rx = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
            if (sdu_rx == NULL) {
                puts("error: unable to allocate rx buffer");
                // ble_l2cap_disconnect(event->accept.chan);
                return 1;
            }
            _coc_chan = event->accept.chan;
            ble_l2cap_recv_ready(_coc_chan, sdu_rx);
            break;
        case BLE_L2CAP_EVENT_COC_DATA_RECEIVED:
            printf("#### BLE_L2CAP_EVENT_COC_DATA_RECEIVED ####\n");
            struct os_mbuf *rxd = event->receive.sdu_rx;
            printf("received %i byte of data\n", (int)OS_MBUF_PKTLEN(rxd));
            os_mbuf_free_chain(rxd);
            rxd = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
            if (rxd == NULL) {
                puts("error: unable to allocate next SDU");
                return 1;
            }
            ble_l2cap_recv_ready(_coc_chan, rxd);
            break;
        default:
            printf("l2cap: server - unkown event (%i)\n", (int)event->type);
            break;
    }

    return 0;
}

static int _on_l2cap_cli_event(struct ble_l2cap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            printf("#### cli BLE_L2CAP_EVENT_COC_CONNECTED ####\n");
            _coc_chan = event->connect.chan;
            break;
        case BLE_L2CAP_EVENT_COC_DISCONNECTED:
            printf("#### cli BLE_L2CAP_EVENT_COC_DISCONNECTED ####\n");
            _coc_chan = NULL;
            break;
        case BLE_L2CAP_EVENT_COC_ACCEPT:
            printf("#### cli BLE_L2CAP_EVENT_COC_ACCEPT ####\n");

            break;
        case BLE_L2CAP_EVENT_COC_DATA_RECEIVED:
            printf("#### cli BLE_L2CAP_EVENT_COC_DATA_RECEIVED ####\n");

            break;
        default:
            printf("l2cap: cli - unkown event (%i)\n", (int)event->type);
            break;
    }

    return 0;
}

static void app_ble_sync_cb(void)
{
    int res;
    res = ble_hs_util_ensure_addr(0);
    assert(res == 0);
    res = ble_hs_id_infer_auto(0, &_own_addr_type);
    assert(res == 0);

    res = ble_l2cap_create_server(CID_CCN, 100, _on_l2cap_server_event, NULL);
    if (res != 0) {
        printf("error: unable to create l2cap server (%i)\n", res);
    }

    (void)res;
}

static void *_nimble(void *arg)
{
    (void)arg;
    /* initialize NimBLE's controller */
    nimble_riot_controller_init();
    /* register the synchronization callback that is triggered once the host has
     * finished its initialization */
    ble_hs_cfg.sync_cb = app_ble_sync_cb;
    /* initialize NimBLE porting layer and the default GATT and GAP services*/
    nimble_port_init();
    nimble_port_run();
    return NULL;
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
            scanlist_print();
            break;
        default:
            printf("unknown event: %i\n", (int)event->type);
            break;
    }

    return 0;
}

static int _on_con_evt(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            puts("\n##### BLE_GAP_EVENT_CONNECT #####");
            printf("handle: 0x%04x\n", (unsigned)event->connect.conn_handle);
            _con_handle = event->connect.conn_handle;
            _coc_con(_con_handle);
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            puts("\n##### BLE_GAP_EVENT_DISCONNECT #####");
            break;
        default:
            printf("_on_con_evt: unknown event: %i\n", (int)event->type);
            break;
    }

    return 0;
}


static void _coc_con(uint16_t con_handle)
{
    if (con_handle == 0) {
        puts("_coc_con: error: con_handle invalid");
        return;
    }
    puts("-> now connecting l2cap channel");
    puts("   1. allocating RX mbuf");

    struct os_mbuf *sdu_rx;
    sdu_rx = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
    int res = ble_l2cap_connect(con_handle, CID_CCN, COC_MAXMTU, sdu_rx,
                                _on_l2cap_cli_event, NULL);
    if (res != 0) {
        printf("_coc_con: error connecting (%i)\n", res);
        return;
    }
    puts("_coc_con: connected\n");
}

static int _on_adv_event(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            puts("\n##### SLAVE: incoming connection ####");
            printf("opening a l2cap channel now\n");
            // uint16_t conn_handle = event->connect.conn_handle;

            break;
        case BLE_GAP_EVENT_DISCONNECT:
            puts("\n##### SLAVE: connection was terminated #####");
            break;
        default:
            printf("_on_adv_event: unkown event: %i\n", (int)event->type);
            break;
    }
    return 0;
}

static int _cmd_scan(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <timeout in ms>\n", argv[0]);
        return 1;
    }

    scanlist_clear();

    struct ble_gap_disc_params params = { 0 };

    uint32_t to = (uint32_t)atoi(argv[1]);
    ble_gap_disc(_own_addr_type, to, &params, _on_scan_evt, NULL);

    return 0;
}

static int _cmd_connect(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s <addr> <max wait>\n", argv[0]);
        return 1;
    }

    ble_addr_t addr;
    /* todo: read addr type from shell */
    addr.type = BLE_ADDR_RANDOM;
    if (_read_addr(&addr, argv[1], strlen(argv[1])) != 0) {
        puts("error: address invalid");
        return 1;
    }

    int max_wait = atoi(argv[2]);

    printf("now connecting to ");
    _print_addr(&addr);

    int res = ble_gap_connect(_own_addr_type, &addr, (int32_t)max_wait,
                              NULL, _on_con_evt, NULL);

    if (res != 0) {
        printf("error: unable to connect (%i)\n", res);
    }
    else {
        puts("connection ok\n");
    }

    return 0;
}

static int _cmd_adv(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage %s <name>\n", argv[0]);
        return 1;
    }

    /* prepare AD data */
    struct ble_hs_adv_fields ad = { 0 };
    ad.flags = (BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
    ad.name = (uint8_t *)argv[1];
    ad.name_len = strlen(argv[1]);
    ad.name_is_complete = 1;
    int res = ble_gap_adv_set_fields(&ad);
    if (res != 0) {
        printf("adv_set_fields failed (%i)\n", res);
        return 1;
    }


    /* start advertising this node */
    struct ble_gap_adv_params adv_params = { 0 };
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    res = ble_gap_adv_start(_own_addr_type, NULL, BLE_HS_FOREVER, &adv_params,
                            _on_adv_event, NULL);
    if (res != 0) {
        printf("adv_start failed (%i)\n", res);
    }

    return 0;
}

static int _cmd_kill(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int res = ble_gap_terminate(_con_handle, BLE_ERR_REM_USER_CONN_TERM);
    if (res != 0) {
        printf("error: unable to terminate connection (0x%02x)\n", res);
    }
    else {
        puts("connection terminated");
    }

    return 0;
}

static int _cmd_send(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <data>\n", argv[0]);
        return 1;
    }

    if (_coc_chan == NULL) {
        puts("error: no open l2cap coc channel");
        return 1;
    }

    struct os_mbuf *sdu_tx = os_mbuf_get_pkthdr(&_coc_mbuf_pool, 0);
    if (!sdu_tx) {
        puts("error: unable to allocate mbuf");
        return 1;
    }

    int res = os_mbuf_append(sdu_tx, argv[1], strlen(argv[1]));
    if (res != 0) {
        printf("error: unable to append data (%i)\n", res);
        return 1;
    }

    res = ble_l2cap_send(_coc_chan, sdu_tx);
    if (res != 0) {
        printf("error: unable to send SDU (%i)\n", res);
    }

    return 0;
}

static const shell_command_t _cmds[] = {
    { "scan", "scan for advertisements", _cmd_scan },
    { "con", "connect to device", _cmd_connect },
    { "adv", "advertise this node", _cmd_adv },
    { "kill", "terminate an open connection or stop advertising", _cmd_kill },
    { "send", "send data over l2cap connection", _cmd_send },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("CCN-lite over BLE demonstrator");

    scanlist_init();

    /* initialize memory pools used for the l2cap channel */
    int res = os_mempool_init(&_coc_mempool, COC_BUF_CNT, COC_MAXMTU, _coc_mem,
                              "sdu_pool");
    if (res != 0) {
        printf("error: unable to initialize SDU mempool (%i)\n", res);
        return 1;
    }
    res = os_mbuf_pool_init(&_coc_mbuf_pool, &_coc_mempool,
                            COC_MAXMTU, COC_BUF_CNT);
    if (res != 0) {
        printf("error: unable to initialize mbuf pool (%i)\n", res);
    }

    /* run nimble thread */
    thread_create(_stack, sizeof(_stack), (THREAD_PRIORITY_MAIN - 1),
                  THREAD_CREATE_STACKTEST, _nimble, NULL, "nimble");

    /* run the RIOT shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(_cmds, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
