/*
 * Copyright (c) 2015-2016 Ken Bannister. All rights reserved.
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
 * @brief       gcoap example
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 *
 * @}
 */

#include <stdio.h>
#include "msg.h"

#include "net/gnrc.h"
#include "net/gcoap.h"
#include "kernel_types.h"
#include "shell.h"
#include "pktcnt.h"
#include "thread.h"

#ifdef MODULE_SIXLOWPAN
#define NETREG_TYPE     (GNRC_NETTYPE_SIXLOWPAN)
#else
#define NETREG_TYPE     (GNRC_NETTYPE_IPV6)
#endif

#define SNIFFER_PRIO    (THREAD_PRIORITY_MAIN - 1)
#define MAIN_QUEUE_SIZE (4)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
static msg_t _sniffer_msg_queue[MAIN_QUEUE_SIZE];
static gnrc_netreg_entry_t entry = GNRC_NETREG_ENTRY_INIT_PID(
                                        GNRC_NETREG_DEMUX_CTX_ALL,
                                        KERNEL_PID_UNDEF
                                    );
static char sniffer_stack[THREAD_STACKSIZE_DEFAULT];


extern int gcoap_cli_cmd(int argc, char **argv);
extern void gcoap_cli_init(void);

static const shell_command_t shell_commands[] = {
    { "coap", "CoAP example", gcoap_cli_cmd },
    { NULL, NULL, NULL }
};

void *_sniffer(void *args)
{
    (void)args;
    entry.target.pid = thread_getpid();
    msg_init_queue(_sniffer_msg_queue, MAIN_QUEUE_SIZE);
    gnrc_netreg_register(NETREG_TYPE, &entry);

    while (1) {
        msg_t msg;
        msg_receive(&msg);
        if (msg.type == GNRC_NETAPI_MSG_TYPE_RCV) {
            pktcnt_log_rx(msg.content.ptr);
            gnrc_pktbuf_release(msg.content.ptr);
        }
        else if (msg.type == GNRC_NETAPI_MSG_TYPE_SND) {
            gnrc_pktbuf_release(msg.content.ptr);
        }
    }
}

int main(void)
{
    /* init pktcnt */
    if (pktcnt_init() != PKTCNT_OK) {
        puts("error: unable to initialize pktcnt");
        return 1;
    }

    thread_create(sniffer_stack, sizeof(sniffer_stack), SNIFFER_PRIO,
                  THREAD_CREATE_STACKTEST, _sniffer, NULL, "pktcnt");

    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    gcoap_cli_init();
    puts("gcoap example app");

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should never be reached */
    return 0;
}
