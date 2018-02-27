/*
 * Copyright (C) 2015 Inria
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
 * @brief       Basic ccn-lite relay example (produce and consumer via shell)
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 *
 * @}
 */

#include <stdio.h>

#ifdef MODULE_TLSF
#include "tlsf-malloc.h"
#endif
#include "msg.h"
#include "shell.h"
#include "ccn-lite-riot.h"
#include "ccnl-pkt-ndntlv.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/pktdump.h"
#include "pktcnt.h"
#include "xtimer.h"

static char _content_generator_stack[THREAD_STACKSIZE_MAIN];
/* main thread's message queue */
#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
static char buf[64];
static unsigned char _out[CCNL_MAX_PACKET_SIZE];

static void _ccnl_content(char *pfx, char *d)
{
    struct ccnl_prefix_s *prefix = ccnl_URItoPrefix(pfx, CCNL_SUITE_NDNTLV, NULL, NULL);
    int offs = CCNL_MAX_PACKET_SIZE;
    strcpy(buf, d);
    int arg_len = ccnl_ndntlv_prependContent(prefix, (unsigned char*)buf, strlen(buf), NULL, NULL, &offs, _out);
    ccnl_prefix_free(prefix);

    unsigned char *olddata;
    unsigned char *data = olddata = _out + offs;

    int len;
    unsigned typ;

    if (ccnl_ndntlv_dehead(&data, &arg_len, (int*) &typ, &len) ||
        typ != NDN_TLV_Data) {
        return;
    }

    struct ccnl_content_s *c = 0;
    struct ccnl_pkt_s *pk = ccnl_ndntlv_bytes2pkt(typ, olddata, &data, &arg_len);
    c = ccnl_content_new(&pk);
    ccnl_content_add2cache(&ccnl_relay, c);
    c->flags |= CCNL_CONTENT_FLAGS_STALE;
}

#ifdef MODULE_TLSF
/* 10kB buffer for the heap should be enough for everyone */
#define TLSF_BUFFER     (10 * 1024 / sizeof(uint32_t))
static uint32_t _tlsf_heap[TLSF_BUFFER];
#endif

static void *_content_generator(void *arg)
{
    (void)arg;
    char _prefix[sizeof("/i3/gasval/000")];
    uint16_t version = 0;

    while (true) {
        sprintf(_prefix, "/i3/gasval/%03u", version++);
        _ccnl_content(_prefix, "3000");
        xtimer_sleep(1);
        version = version % 1000;
    }

    return NULL;
}

int main(void)
{
    uint16_t src_len = 8;
#ifdef MODULE_TLSF
    tlsf_create_with_pool(_tlsf_heap, sizeof(_tlsf_heap));
#endif
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    puts("Basic CCN-Lite example");

    ccnl_core_init();

    ccnl_start();

    /* get the default interface */
    gnrc_netif_t *netif = gnrc_netif_iter(NULL);

    gnrc_netapi_set(netif->pid, NETOPT_SRC_LEN, 0, &src_len, sizeof(src_len));

    /* init pktcnt */
    if (pktcnt_init() != PKTCNT_OK) {
        puts("error: unable to initialize pktcnt");
        return 1;
    }

    /* set the relay's PID, configure the interface to use CCN nettype */
    if (ccnl_open_netif(netif->pid, GNRC_NETTYPE_CCN) < 0) {
        puts("Error registering at network interface!");
        return -1;
    }

#ifdef MODULE_NETIF
    gnrc_netreg_entry_t dump = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                          gnrc_pktdump_pid);
    gnrc_netreg_register(GNRC_NETTYPE_CCN_CHUNK, &dump);
#endif
    thread_create(_content_generator_stack, sizeof(_content_generator_stack),
                  THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  _content_generator, NULL, "content_generator");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
