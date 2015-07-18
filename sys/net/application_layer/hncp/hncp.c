/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_dncp
 * @{
 *
 * @file
 * @brief       Home Network Control Protocol (HNCP) Implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "net/hncp.h"
#include "net/dncp.h"
#include "net/hncp/tlvs.h"

/* TODO: need src address and src port */
static void dispatch(dncp_tlv_t *tlv)
{
    /* DNCP type? */
    if (tlv->type > 0 && tlv->type <= DNCP_TLV_TYPE_TRUST_VERDICT) {
        dncp_dispatch(tlv)
    }
}

static void parse_packet(ng_pktsnip_t *snip)
{
    do {
        /* read TLVs from incoming packet, TODO: adjust for other transports */
        dncp_tlv_t *tlv = (dncp_tlv_t *)snip->data;
        dispatch(tlv);
    } while ();
}

static void *event_loop(void *arg)
{
    msg_t msg, reply;
    mst_t msg_queue[HNCP_MESSAGE_QUEUE_SIZE];

    /* prepare reply message */
    reply.type = NG_NETAPI_MSG_TYPE_ACK;
    reply.content.value = (uint32_t)-ENOTSUP;
    /* initialize the message queue */
    msg_init_queue(msg_queue, HNCP_MESSAGE_QUEUE_SIZE);

    /* receive incoming network packets */
    while (1) {
        msg_receive(&msg);

        switch (msg.type) {
            case NG_NETAPI_MSG_TYPE_RCV:
                ng_pktsnip_t *snip = (ng_pktsnip_t *)msg.content.value;
                parse_packet(snip);
                ng_pktbuf_release(snip);
                break;
            case NG_NETAPI_MSG_TYPE_SET:
            case NG_NETAPI_MSG_TYPE_GET:
                msg_reply(&msg, &reply);
                break;
            default:
                DEBUG("[hncp] Received unsupported message type\n");
                break;
        }
    }


    /* should not be reached */
    return NULL;
}


void hncp_init(ng_nettype_t transport, uint16_t port,
               char *stack, int stacksize, char prio)
{
    /* check transport */

    /* create the HNCP thread */
    thread_create(stack, stacksize, prio, THREAD_CREATE_STACKTEST,
                  event_loop, &port, "hncp");

}

void hncp_hash(uint8_t *hash, const uint8_t *data, size_t len)
{
    uint8_t res[16];
    md5(res, data, len);
    memcpy(hash, res, 4);   /* TODO; was it the first or last 4 byte of the hash? */
}

int hncp_send(ng_ipv6_addr_t *addr, uint16_t port, uint8_t *data, size_t len)
{
    ng_pktsnip_t *payload, *trans, *net;
    uint8_t p[2];
    p[0] = port >> 8;
    p[1] = (uint8_t)port;

    /* allocate payload in packet buffer */
    payload = ng_pktbuffer_add(NULL, data, len, NG_NETTYPE_UNDEF);
    if (payload == NULL) {
        /* error */
        return -1;
    }
    /* TODO: adjust to do something else than UDP */
    trans = ng_udp_hdr_build(payload, p, 2, p, 2);
    if (trans == NULL) {
        /* error */
        ng_pktbuf_release(payload);
        return -1;
    }
    net = ng_ipv6_hdr_build(trans, NULL, 0, (uint8_t *)&addr, sizeof(addr[0]));
    if (net == NULL) {
        /* next error */
        ng_pktbuf_release(trans);
        return -1;
    }
    /* forward packet to the transport layer */
    if (!ng_netapi_dispatch_send(trans->type, NG_NETREG_DEMUX_CTX_ALL, pkt)) {
        DEBUG("udp: cannot send packet: network layer not found\n");
        ng_pktbuf_release(net);
    }
}
