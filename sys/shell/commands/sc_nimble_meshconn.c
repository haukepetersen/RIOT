/*
 * Copyright (C) 2021 Freie Universität Berlin
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
 * @brief       Shell commands to control the NimBLE netif meshconn connection
 *              manager
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>

#include "net/bluetil/addr.h"
#include "nimble_meshconn.h"

// static nimble_meshconn_sub_t _sub;

// static void _print_state(const char *msg, const uint8_t *addr)
// {
//     printf("[meshconn] %s ", msg);
//     bluetil_addr_print(addr);
//     puts("");
// }

// static void _on_meshconn_event(nimble_meshconn_event_t event,
//                                int handle, const uint8_t *addr)
// {
//     (void)handle;

//     switch (event) {
//         case NIMBLE_MESHCONN_RECONNECT:
//             _print_state("RECONNECT", addr);
//             break;
//         case NIMBLE_MESHCONN_CONN_UPLINK:
//             _print_state("CONN_UPLINK", addr);
//             break;
//         case NIMBLE_MESHCONN_CONN_DOWNLINK:
//             _print_state("CONN_DOWNLINK", addr);
//             break;
//         case NIMBLE_MESHCONN_CLOSE_UPLINK:
//             _print_state("CLOSE_UPLINK", addr);
//             break;
//         case NIMBLE_MESHCONN_CLOSE_DOWNLINK:
//             _print_state("CLOSE_DOWNLINK", addr);
//             break;
//         default:
//             /* nothing to be done in this case */
//             break;
//     }
// }

void sc_nimble_meshconn_init(void)
{
    // _sub.cb = _on_meshconn_event;
    // nimble_meshconn_subscribe(&_sub);
}

int _nimble_meshconn_handler(int argc, char **argv)
{
    if ((argc < 2)) {
        printf("usage: %s <create [netID]|join [netID]|leave>\n", argv[0]);
        return 0;
    }

    if (strncmp(argv[1], "create", 6) == 0) {
        if (argc < 3) {
            puts("err: unable to parse netID\n");
            return 1;
        }
        uint32_t netid = (uint32_t)atol(argv[2]);

        int res = nimble_meshconn_create(netid);
        if (res == 0) {
            puts("success: created BLE network\n");
        }
        else {
            puts("err: unable to create network\n");
        }
    }
    else if (strncmp(argv[1], "join", 4) == 0) {
        if (argc < 3) {
            puts("err: unable to parse netID\n");
            return 1;
        }
        uint32_t netid = (uint32_t)atol(argv[2]);

        /* join the BLE network using default parameters */
        int res = nimble_meshconn_join(netid);
        if (res == 0) {
            printf("success: connecting to network %u\n", (unsigned)netid);
        }
        else {
            puts("err: unable to join network");
        }
    }
    else if (strncmp(argv[1], "leave", 5) == 0) {
        int res = nimble_meshconn_leave();
        if (res == 0) {
            puts("success: not connected to any network anymore");
        }
        else {
            puts("err: failed to leave network");
        }
    }
    else {
        puts("err: unable to parse command");
    }

    return 0;
}
