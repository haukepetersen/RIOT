/*
 * Copyright (C) 2017 Freie Universität Berlin
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
 * @brief       Shell commands for the rdcli module
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>

#include "net/rdcli.h"
#include "nanocoap.h"
#include "net/rdcli_config.h"
#include "net/rdcli_common.h"

static int make_sock_ep(sock_udp_ep_t *ep, const char *addr)
{
    if (ipv6_addr_from_str((ipv6_addr_t *)&ep->addr, addr) == NULL) {
        return -1;
    }
    ep->family  = AF_INET6;
    ep->netif   = SOCK_ADDR_ANY_NETIF;
    return 0;
}

int _rdcli_handler(int argc, char **argv)
{
    int res;

    if (argc > 1 && (strcmp(argv[1], "reg") == 0)) {
        if (argc > 2) {
            sock_udp_ep_t remote;
            if (make_sock_ep(&remote, argv[2]) < 0) {
                printf("error: unable to parse address\n");
                return 1;
            }
            if (argc > 3) {
                remote.port = (uint16_t)atoi(argv[3]);
            }
            else {
                remote.port = RDCLI_SERVER_PORT;
            }
            res = rdcli_register(&remote);
        }
        else {
            res = rdcli_register(NULL);
        }
        if (res != RDCLI_OK) {
            puts("error during registration");
        }
        else {
            puts("registration successful\n");
            rdcli_dump_status();
        }
    }
    else if (argc > 1 && (strcmp(argv[1], "update") == 0)) {
        res = rdcli_update();
        if (res == RDCLI_OK) {
            puts("RD update successful");
        }
        else if (res == RDCLI_NORD) {
            puts("error: not associated with any RD");
        }
        else if (res == RDCLI_TIMEOUT) {
            puts("error: unable to reach RD - dropped association");
        }
        else {
            puts("error: RD update failed");
        }
    }
    else if (argc > 1 && (strcmp(argv[1], "rem") == 0)) {
        res = rdcli_remove();
        if (res == RDCLI_OK) {
            puts("node successfully removed from RD");
        }
        else if (res == RDCLI_NORD) {
            puts("error: not associated with any RD");
        }
        else if (res == RDCLI_TIMEOUT) {
            puts("error: unable to reach RD - remove association only locally");
        }
        else {
            puts("error: unable to remove node from RD");
        }
    }
    else if (argc > 1 && (strcmp(argv[1], "info") == 0)) {
        rdcli_dump_status();
    }
    else {
        printf("usage: %s <reg [addr [port]]|update|rem|info>\n", argv[0]);
        return 1;
    }

    return 0;
}
