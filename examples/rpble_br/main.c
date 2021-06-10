/*
 * Copyright (C) 2015 Freie Universität Berlin
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
 * @brief       Example application for demonstrating the RIOT network stack
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "shell.h"
#include "msg.h"
#include "nimble_rpble.h"
#include "net/bluetil/addr.h"

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static void _on_ble_evt(int handle, nimble_netif_event_t event,
                        const uint8_t *addr)
{
    switch (event) {
        case NIMBLE_NETIF_CONNECTED_SLAVE:
            printf("[ble] ");
            bluetil_addr_print(addr);
            printf(" - child connected (h:%i)\n", handle);
            break;
        case NIMBLE_NETIF_CLOSED_SLAVE:
            printf("[ble] ");
            bluetil_addr_print(addr);
            printf(" - child disconnect (h:%i)\n", handle);
            break;
        default:
            /* no need for anything */
            break;
    }
}

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("RIOT border router example application");

    /* register out custom event callback to get logging information about
     * BLE link state */
    nimble_rpble_eventcb(_on_ble_evt);

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
