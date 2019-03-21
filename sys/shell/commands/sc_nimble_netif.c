/*
 * Copyright (C) 2019 Freie Universität Berlin
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
 * @brief       Shell commands for NimBLE netif adaption
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "net/ble.h"
#include "net/bluetil/addr.h"
#include "nimble_riot.h"
#include "nimble_netif.h"
#include "host/ble_hs.h"

#ifdef MODULE_NIMBLE_SCANNER
#include "nimble_scanner.h"
#endif
#ifdef MODULE_NIMBLE_NETIF_AUTOCONN
#include "nimble_netif_autoconn.h"
#endif

int _nimble_netif_handler(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    int res;

    /* my address */
    uint8_t addr[BLE_ADDR_LEN];
    res = ble_hs_id_copy_addr(nimble_riot_own_addr_type, addr, NULL);
    assert(res == 0);

    printf("BLE address: ");
    bluetil_addr_print(addr);
    puts("");

#ifdef MODULE_NIMBLE_SCANNER
    printf("Scan status: ");
    if (nimble_scanner_status() == NIMBLE_SCANNER_SCANNING) {
        puts("SCANNING");
    }
    else {
        puts("-");
    }
#endif

    printf("Adv status:  ");
    if (nimble_netif_get_adv_ctx() != NULL) {
        puts("ADVERTISING");
    }
    else {
        puts("-");
    }

#ifdef MODULE_NIMBLE_NETIF_AUTOCONN
    printf("Free slots: %u\n", nimble_netif_autoconn_free_slots());
#endif

    puts("Connections:");
    nimble_netif_print_conn_info();

    return 0;
}
