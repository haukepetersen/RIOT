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

#include <errno.h>

#include "net/ng_netapi.h"
#include "net/ng_pkt.h"
#include "net/ng_pktbuf.h"
#include "net/hncp.h"
#include "net/dncp.h"
#include "net/hncp/tlvs.h"
#include "hashes/md5.h"
#include "net/ng_ipv6.h"
#include "net/ng_udp.h"
#include "net/ng_nettype.h"
#include "byteorder.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

dncp_profile_t dncp_profile;

/* TODO: need src address and src port */
static void dispatch(dncp_tlv_t *tlv)
{
    /* DNCP type? */
    if (byteorder_ntohs(tlv->type) > 0 && byteorder_ntohs(tlv->type) <= DNCP_TLV_TYPE_TRUST_VERDICT) {
        dncp_dispatch(tlv);
    }
}

static void parse_packet(ng_pktsnip_t *snip)
{
    size_t len = snip->size;
    do {
        /* read TLVs from incoming packet, TODO: adjust for other transports */
        dncp_tlv_t *tlv = (dncp_tlv_t *)snip->data;
        len -= sizeof(tlv->type) + sizeof(tlv->length) + byteorder_ntohs(tlv->length);
        dispatch(tlv);
    } while (len);
}

static void *event_loop(void *arg)
{
    (void) arg;

    msg_t msg, reply;
    msg_t msg_queue[HNCP_MESSAGE_QUEUE_SIZE];
    ng_pktsnip_t *snip;

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
                snip = (ng_pktsnip_t *)msg.content.value;
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
    (void) transport; /* TODO: for now unchecked */

    dncp_profile.transport = NG_NETTYPE_UDP;
    dncp_profile.port = HNCP_PORT;
    dncp_profile.node_id_len = HNCP_NODE_ID_LEN;
    dncp_profile.cb_hash = hncp_hash;
    dncp_profile.mcast.u8[0] = 0xff;
    dncp_profile.mcast.u8[1] = 0x02;
    dncp_profile.mcast.u8[14] = 0x88;
    dncp_profile.mcast.u8[15] = 0x08;

    /* create the HNCP thread */
    thread_create(stack, stacksize, prio, CREATE_STACKTEST,
                  event_loop, &port, "hncp");

}

void hncp_hash(uint8_t *hash, const uint8_t *data, size_t len)
{
    uint8_t res[16];
    md5(res, data, len);
    memcpy(hash, res, 4);   /* TODO; was it the first or last 4 byte of the hash? */
}

void _write_tlv_to_buffer(uint8_t *buf, dncp_tlv_t *tlv, uint8_t *value)
{
    memcpy(buf, (uint8_t *) tlv, sizeof(*tlv));
    memcpy(buf + sizeof(*tlv), value, byteorder_ntohs(tlv->length));
    return ;
}

int hncp_req_node(uint8_t *node_identifier)
{
    size_t len = sizeof(uint16_t) * 2 + dncp_profile.node_id_len;
    uint8_t buf[len];
    dncp_tlv_t tlv = {
        .type = byteorder_htons(DNCP_TLV_TYPE_REQ_NODE_STATE),
        .length = byteorder_htons(dncp_profile.node_id_len),
    };

    uint8_t val[dncp_profile.node_id_len];
    memset(val, 0, dncp_profile.node_id_len);

    memcpy(val + dncp_profile.node_id_len - strlen((char *) node_identifier),
            node_identifier, dncp_profile.node_id_len);

    _write_tlv_to_buffer(buf, &tlv, val);

	hncp_send(&dncp_profile.mcast, dncp_profile.port, buf, len);
    return 0;
}

int hncp_send(ng_ipv6_addr_t *addr, uint16_t port, uint8_t *data, size_t len)
{
    ng_pktsnip_t *payload, *trans, *net;
    ng_udp_hdr_t *udp;

    uint8_t p[2];
    p[0] = (uint8_t)port;
    p[1] = port >> 8;

    /* allocate payload in packet buffer */
    payload = ng_pktbuf_add(NULL, data, len, NG_NETTYPE_UNDEF);
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
    udp = (ng_udp_hdr_t *) trans->data;
    udp->length = byteorder_htons(ng_pkt_len(trans));
    net = ng_ipv6_hdr_build(trans, NULL, 0, (uint8_t *)addr, sizeof(*addr));
    if (net == NULL) {
        /* next error */
        ng_pktbuf_release(trans);
        return -1;
    }
    /* forward packet to the transport layer */
    if (!ng_netapi_dispatch_send(net->type, NG_NETREG_DEMUX_CTX_ALL, net)) {
        DEBUG("udp: cannot send packet: network layer not found\n");
        ng_pktbuf_release(net);
        return -1;
    }

    return 0;
}
