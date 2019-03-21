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
#include <string.h>

#include "shell.h"
#include "assert.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/util/util.h"

#include "net/bleutil/ad.h"
#include "net/facemaster.h"
#include "net/faceimp/nimble.h"

#include "scanlist.h"

/* time to wait when connecting before timing out (in [ms])*/
#define MAX_WAIT            (1000U)

/* name advertised by this node */
#define NODE_NAME           "Horstie"

// TMP: so far we only allow for 2 connections, one master, one slave...
static faceimp_nimble_t _server_imp;
static faceimp_nimble_t _client_imp;

/* TODO: move to some kind of NimBLE specific RIOT helper module */
static void _print_addr(ble_addr_t *addr)
{
    printf("%02x", (int)addr->val[5]);
    for (int i = 4; i >= 0; i--) {
        printf(":%02x", addr->val[i]);
    }
    switch (addr->type) {
        case BLE_ADDR_PUBLIC:       printf(" (PUBLIC)\n");   break;
        case BLE_ADDR_RANDOM:       printf(" (RANDOM)\n");   break;
        case BLE_ADDR_PUBLIC_ID:    printf(" (PUB_ID)\n");   break;
        case BLE_ADDR_RANDOM_ID:    printf(" (RAND_ID)\n");  break;
        default:                    printf(" (UNKNOWN)\n");  break;
    }
}

static int _advertise(faceimp_nimble_t *imp)
{
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bleutil_ad_t ad;
    bleutil_ad_init_with_flags(&ad, buf, sizeof(buf), BLEUTIL_AD_FLAGS_DEFAULT);
    bleutil_ad_add_name(&ad, NODE_NAME);

    struct ble_gap_adv_params adv_params = { 0 };
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    return faceimp_nimble_accept(imp, ad.buf, (int)ad.pos, &adv_params);
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
            break;
    }

    return 0;
}

static void _on_nim_imp_event(faceimp_nimble_t *nim_imp,
                             faceimp_nimble_event_t event)
{
    switch (event) {
        case FACEIMP_NIMBLE_CONNECTED:
            printf("NimBLE-Imp %p: CONNECTED\n",  (void *)nim_imp);
            break;
        case FACEIMP_NIMBLE_DISCONNECTED:
            printf("NimBLE-Imp %p: DISCONNECTED\n", (void *)nim_imp);
            if (nim_imp == &_server_imp) {
                _advertise(&_server_imp);
            }
            break;
        case FACEIMP_NIMBLE_ABORT:
            printf("NimBLE-Imp %p: ABORTED\n", (void *)nim_imp);
            break;
        default:
            /* do nothing */
            break;
    }
}


static int _dummy_on_up(unsigned face, uint16_t flags)
{
    (void)flags;
    printf("Face #%u is UP\n", face);
    return FACEMASTER_OK;
}

static int _dummy_on_down(unsigned face)
{
    printf("Face #%u is DOWN\n", face);
    return FACEMASTER_OK;
}

static int _dummy_on_data(unsigned face, void *data, size_t len)
{
    printf("Face #%u: received DATA\n", face);
    printf("     %u bytes -> %s\n", len, (const char *)data);
    return FACEMASTER_OK;
}

static const facemaster_hooks_t _dummy_hooks = {
    .up = _dummy_on_up,
    .down = _dummy_on_down,
    .data = _dummy_on_data,
};

static int _cmd_ble_info(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    // TODO: print connection infos
    puts("TODO: nothing done, yet");

    return 0;
}

static int _cmd_ble_scan(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <timeout in ms>\n", argv[0]);
        return 1;
    }

    uint32_t to = (uint32_t)atoi(argv[1]);
    struct ble_gap_disc_params params = { 0 };
    uint8_t atype;

    scanlist_clear();
    ble_hs_id_infer_auto(0, &atype);
    ble_gap_disc(atype, to, &params, _on_scan_evt, NULL);

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
    if (_client_imp.l2cap_coc != NULL) {
        puts("error: already connected\n");
        return 1;
    }

    scanlist_t *e = scanlist_get((unsigned)atoi(argv[1]));
    if (e == NULL) {
        puts("error: invalid scanlist entry # given");
        return 1;
    }

    printf("Connecting to ");
    _print_addr(&e->addr);

    // TODO: configure connection parameters, use default ones until then
    // struct ble_gap_conn_params conn_params = { 0 };
    int res = faceimp_nimble_connect(&_client_imp, &e->addr,
                                     NULL, MAX_WAIT);
    if (res != FACEIMP_OK) {
        printf("error: failed to start connection procedure (%i)\n", res);
        return 1;
    }

    return 0;
}

static int _cmd_facemaster_send(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s <face> <data>\n", argv[0]);
        return 1;
    }

    unsigned face = (unsigned)atoi(argv[1]);
    size_t len = strlen(argv[2]) + 1; /* include th '\0' terminator */

    int res = facemaster_send(face, argv[2], len);
    if (res == FACEMASTER_NOFACE) {
        printf("error: given face ('#%u') is invalid\n", face);
    }
    else if (res != FACEMASTER_OK) {
        puts("error: unable to send data using the defined face");
    }

    return 0;
}

static int _cmd_facemaster_kill(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <face>\n", argv[0]);
        return 1;
    }

    unsigned face = (unsigned)atoi(argv[1]);
    int res = facemaster_face_kill(face);
    if (res != FACEMASTER_OK) {
        printf("error: unable to kill face #%u (%i)\n", face, res);
        return 1;
    }

    printf("Initiated termination of face #%u\n", face);

    return 0;
}

static const shell_command_t _cmds[] = {
    { "ble_info", "show all active faces", _cmd_ble_info },
    { "ble_scan", "discover neighboring nodes", _cmd_ble_scan },
    { "ble_conn", "connect to a selected node", _cmd_ble_conn },
    { "face_send", "send data on given face", _cmd_facemaster_send },
    { "face_kill", "terminate a face", _cmd_facemaster_kill },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("CCN-lite over BLE demonstrator");

    /* create a list for keeping scan results */
    scanlist_init();

    /* initialize facemaster */
    // TODO: move to auto_init
    facemaster_init();
    // TODO: for testing, we use a dummy ICN stack
    facemaster_hook(&_dummy_hooks);

    /* we use NimBLE Imps, so initialize their infrastructure */
    faceimp_nimble_init();
    faceimp_nimble_eventcb(_on_nim_imp_event);

    /* make node accept incoming face connections */
    int res = _advertise(&_server_imp);
    if (res != FACEIMP_OK) {
        puts("error: unable to start advertising the NimBLE Imp!");
        return 1;
    }

    /* run the RIOT shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(_cmds, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
