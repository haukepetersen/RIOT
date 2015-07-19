/*
 * Copyright (C) 2015 Freie Universität Berlin
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
 * @brief       Shell commands for interacting with homenet
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#include <stdint.h>
#include <stdio.h>

#include <net/hncp.h>
#include <net/dncp.h>

#include "net/ng_nettype.h"

static char _stack[HNCP_STACKSIZE_DEFAULT];

extern int _hncp_init(int argc, char **argv)
{
    (void)argc;
    (void)argv;

	hncp_init(NG_NETTYPE_UDP, 1337, _stack, sizeof(_stack), HNCP_PRIO_DEFAULT);
    return 0;
}

extern int _hncp_list(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return 0;
}

extern int _hncp_req_node(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (argc == 2) {
        hncp_req_node((uint8_t *) argv[1]);
    }
    else {
        return -1;
    }

    return 0;
}

extern int _hncp_req_net(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return 0;
}
