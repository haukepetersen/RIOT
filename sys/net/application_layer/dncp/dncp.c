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
 * @brief       Distributed Neighbor Consensus Protocol (DNCP) implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "net/dncp.h"
#include "net/dncp/tlvs.h"

void dncp_init(dncp_profile_t *profile)
{
	(void) profile;

}


void dncp_dispatch(dncp_tlv_t *tlv)
{
    switch (byteorder_ntohs(tlv->type)) {
        case DNCP_TLV_TYPE_REQ_NET_STATE:


            break;
        case DNCP_TLV_TYPE_REQ_NODE_STATE:

            break;
        case DNCP_TLV_TYPE_NET_STATE:

            break;
        case DNCP_TLV_TYPE_NODE_STATE:


            break;
        case DNCP_TLV_TYPE_NODE_EP:
        case DNCP_TLV_TYPE_FRAG_COUNT:
        case DNCP_TLV_TYPE_NEIGHBOR:
        case DNCP_TLV_TYPE_KEEP_ALIVE_INT:
        case DNCP_TLV_TYPE_TRUST_VERDICT:
        default:
            /* TODO: not supported, yet */
            break;
    }
}
