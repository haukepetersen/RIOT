/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_rdcli
 * @{
 *
 * @file
 * @brief       Simplified CoAP resource directory client implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <string.h>
#include <stdio.h>

#include "fmt.h"
#include "mutex.h"
#include "thread_flags.h"

#include "net/gcoap.h"
#include "net/ipv6/addr.h"
#include "net/rdcli.h"
#include "net/rdcli_common.h"
#include "net/rdcli_config.h"


#define ENABLE_DEBUG    (0)
#include "debug.h"

#define FLAG_SUCCESS        (0x0001)
#define FLAG_TIMEOUT        (0x0002)
#define FLAG_ERR            (0x0004)
#define FLAG_MASK           (FLAG_SUCCESS | FLAG_TIMEOUT | FLAG_ERR)

#define BUFSIZE             (512U)

typedef struct {
    char res_loc[RDCLI_RES_LOC_LEN];
    sock_udp_ep_t remote;
} rd_entry_t;

static rd_entry_t active_rd;

static mutex_t lock = MUTEX_INIT;
static volatile thread_t *t_waiting;

static uint8_t buf[BUFSIZE];

/* allocate the default UDP endpoint for reaching RD servers */
static const sock_udp_ep_t default_remote = {
    .family    = AF_INET6,
    .netif     = SOCK_ADDR_ANY_NETIF,
    .addr      = RDCLI_SERVER_ADDR ,
    .port      = RDCLI_SERVER_PORT
};

static size_t parse_location_path(coap_pkt_t *pdu)
{
    size_t pos = 0;
    coap_opt_t opt;
    uint8_t *tmp = pdu->options;

    tmp = coap_find_opt(pdu, tmp, &opt, COAP_OPT_LOCATION_PATH);
    if (!tmp) {
        return 0;
    }
    do {
        if ((pos + opt.len) >= RDCLI_RES_LOC_LEN) {
            return 0;
        }

        /* save part of location */
        active_rd.res_loc[pos++] = '/';
        memcpy(active_rd.res_loc + pos, opt.val, opt.len);
        pos += opt.len;

        /* try to find more location options */
        tmp = coap_find_opt(pdu, tmp, &opt, 0);
    } while (tmp);

    return pos;
}

static void on_register(unsigned req_state, coap_pkt_t* pdu, sock_udp_ep_t *remote)
{
    thread_flags_t flag = FLAG_ERR;

    if ((req_state == GCOAP_MEMO_RESP) && (pdu->hdr->code == COAP_CODE_CREATED)) {
        /* read the location header and save the RD details on success */
        if (parse_location_path(pdu) > 0) {
            memcpy(&active_rd.remote, remote, sizeof(active_rd.remote));
            flag = FLAG_SUCCESS;
        }
        else {
            DEBUG("on_register: unable to parse location path\n");
            /* reset RD entry */
            active_rd.res_loc[0] = 0;
        }
    }
    else if (req_state == GCOAP_MEMO_TIMEOUT) {
        DEBUG("on_register: timeout while trying to reach the RD\n");
        flag = FLAG_TIMEOUT;
    }

    thread_flags_set((thread_t *)t_waiting, flag);
}

static void on_update_remove(unsigned req_state, coap_pkt_t *pdu, uint8_t code)
{
    thread_flags_t flag = FLAG_ERR;

    if ((req_state == GCOAP_MEMO_RESP) && (pdu->hdr->code == code)) {
        flag = FLAG_SUCCESS;
    }
    else if (req_state == GCOAP_MEMO_TIMEOUT) {
        flag = FLAG_TIMEOUT;
    }

    thread_flags_set((thread_t *)t_waiting, flag);
}

static void on_update(unsigned req_state, coap_pkt_t *pdu, sock_udp_ep_t *remote)
{
    (void)remote;
    on_update_remove(req_state, pdu, COAP_CODE_CHANGED);
}

static void on_remove(unsigned req_state, coap_pkt_t *pdu, sock_udp_ep_t *remote)
{
    (void)remote;
    on_update_remove(req_state, pdu, COAP_CODE_DELETED);
}

int rdcli_register(sock_udp_ep_t *remote)
{
    int retval = RDCLI_OK;
    coap_pkt_t pkt;

    /* fall back to the default RD server address if none was given */
    if (!remote) {
        remote = (sock_udp_ep_t *)&default_remote;
    }

    mutex_lock(&lock);

    /* reset existing RD entry */
    active_rd.res_loc[0] = 0;

    /* build CoAP request packet */
    int res = gcoap_req_init(&pkt, buf, sizeof(buf), COAP_METHOD_POST, "/rd");
    if (res < 0) {
        goto rdcli_register;
    }
    /* set some packet options and write query string */
    coap_hdr_set_type(pkt.hdr, COAP_TYPE_CON);
    res = rdcli_common_add_qstring(&pkt);

    /* add the resource description as payload */
    res = gcoap_get_resource_list(pkt.payload, pkt.payload_len,
                                      COAP_FORMAT_LINK);
    if (res < 0) {
        retval = RDCLI_ERR;
        goto rdcli_register;
    }

    /* finish up the packet */
    ssize_t pkt_len = gcoap_finish(&pkt, res, COAP_FORMAT_LINK);

    /* send out the request */
    res = gcoap_req_send2(buf, pkt_len, remote, on_register);
    if (res < 0) {
        retval = RDCLI_ERR;
        goto rdcli_register;
    }
    /* wait on the sync lock, will be unlocked by the `on_register` handler */
    t_waiting = sched_active_thread;
    thread_flags_t flags = thread_flags_wait_any(FLAG_MASK);
    if (flags & FLAG_TIMEOUT) {
        retval = RDCLI_TIMEOUT;
    }
    else if (flags & FLAG_ERR) {
        retval = RDCLI_ERR;
    }

rdcli_register:
    mutex_unlock(&lock);
    return retval;
}

static int update_remove(unsigned code, gcoap_resp_handler_t handle)
{
    coap_pkt_t pkt;

    if (active_rd.res_loc[0] == 0) {
        return RDCLI_NORD;
    }

    /* build CoAP request packet */
    DEBUG("update_remove: path: %s\n", active_rd.res_loc);
    int res = gcoap_req_init(&pkt, buf, sizeof(buf), code, active_rd.res_loc);
    if (res < 0) {
        return RDCLI_ERR;
    }
    coap_hdr_set_type(pkt.hdr, COAP_TYPE_CON);
    ssize_t pkt_len = gcoap_finish(&pkt, 0, COAP_FORMAT_NONE);

    /* send request */
    gcoap_req_send2(buf, pkt_len, &active_rd.remote, handle);

    /* synchronize response */
    t_waiting = sched_active_thread;
    thread_flags_t flags = thread_flags_wait_any(FLAG_MASK);
    if (flags & FLAG_ERR) {
        return RDCLI_ERR;
    }
    if (flags & FLAG_TIMEOUT) {
        return RDCLI_TIMEOUT;
    }

    return RDCLI_OK;
}

int rdcli_update(void)
{
    mutex_lock(&lock);
    int res = update_remove(COAP_METHOD_POST, on_update);
    if (res == FLAG_TIMEOUT) {
        /* in case we are not able to reach the RD, we drop the association */
        active_rd.res_loc[0] = 0;
    }
    mutex_unlock(&lock);
    return res;
}

int rdcli_remove(void)
{
    mutex_lock(&lock);
    int res = update_remove(COAP_METHOD_DELETE, on_remove);
    if (res != FLAG_ERR) {
        /* in any other case than internal errors we drop the RD association */
        active_rd.res_loc[0] = 0;
    }
    mutex_unlock(&lock);
    return res;
}

void rdcli_dump_status(void)
{
    puts("CoAP RD connection status:");

    if (active_rd.res_loc[0] == 0) {
        puts("  --- not registered with any RD ---");
    }
    else {
        /* get address string */
        char addr[IPV6_ADDR_MAX_STR_LEN];
        ipv6_addr_to_str(addr, (ipv6_addr_t *)&active_rd.remote.addr, sizeof(addr));

        printf("      host: coap://[%s]:%i\n", addr, (int)active_rd.remote.port);
        printf("  location: %s\n", active_rd.res_loc);
    }

}
