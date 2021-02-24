/*
 * Copyright (C) 2020-2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_shell_commands
 * @{
 *
 * @file
 * @brief       Shell commands to control Gorm
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net/gorm/netif.h"

#define ADV_INTERVAL_MS_DFLT        100U

static int _is_init = 0;

static gorm_gap_adv_ctx_t _adv_ctx;
static gorm_ctx_t _con;

static int _is_cmd(const char *arg, const char *cmd, size_t len)
{
    return memcmp(arg, cmd, len) == 0;
}

static void _info(void)
{
    printf("Gorms Status:");
    if (IS_ACTIVE(MODULE_GORM_NETIF)) {
        gorm_netif_dump_state();
    }
    puts("--END--");
}

static void _adv(int argc, char **argv)
{
    if ((argc < 3) || _is_cmd(argv[2], "help", 4)) {
        printf("usage: %s %s <help|start [name [interval]]|stop>\n", argv[0], argv[1]);
    }
    else if (_is_cmd(argv[2], "start", 5)) {
        // TODO check if the local context is already in use

        char *name = NULL;
        uint16_t itvl = ADV_INTERVAL_MS_DFLT;
        if (argc >= 4) {
            name = argv[3];
        }
        if (argc >= 5) {
            itvl = (uint16_t)atoi(argv[4]);
        }

        /* build advertising data */
        bluetil_ad_t ad = {.buf = _adv_ctx.ad, .pos = 0, .size = BLE_ADV_PDU_LEN };
        // bluetil_ad_add_flags(&ad, BLUETIL_AD_FLAGS_DEFAULT);
        bluetil_ad_add_flags(&ad, BLE_GAP_FLAG_BREDR_NOTSUP);
        if (name) {
            bluetil_ad_add_name(&ad, name);
        }

        int res = gorm_gap_adv_init(&_adv_ctx, itvl, ad.buf, ad.pos, NULL, 0);
        if (res != 0) {
            printf("error: unable to initialize advertising context (%i)\n", res);
            return;
        }

        res = gorm_gap_adv_start(&_con, &_adv_ctx, GORM_GAP_CONN_UND);
        // res = gorm_gap_adv_start(&_con, &_adv_ctx, GORM_GAP_CONN_NON);
        if (res != 0) {
            printf("error: unable to start advertising (%i)\n", res);
            return;
        }
        printf("started advertising\n");
    }
    else if (_is_cmd(argv[2], "stop", 4)) {
        printf("stop advertising\n");
    }
}



int _gorm_handler(int argc, char **argv)
{
    if (_is_init == 0) {
        gorm_ctx_init(&_con);
        _is_init = 1;
    }

    if ((argc < 2) || _is_cmd(argv[1], "help", 4)) {
        printf("usage: %s [help|info|adv]\n", argv[0]);
    }
    else if (_is_cmd(argv[1], "info", 4)) {
        _info();
    }
    else if (_is_cmd(argv[1], "adv", 3)) {
        _adv(argc, argv);
    }
    else {
        puts("invalid command");
    }

    return 0;
}
