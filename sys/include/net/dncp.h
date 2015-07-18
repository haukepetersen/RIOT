/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_dncp DNCP
 * @ingroup     net
 * @brief       Distributed Node Consensus Protocol (DNCP) Implementation
 *
 * The Distributed Node Consensus Protocol (DNCP) is "a generic state
 * synchronization protocol which uses Trickle and Merkle trees. DNCP leaves
 * some details unspecified or provides alternative options. Therefore, only
 * profiles which specify those missing parts define actual implementable DNCP-
 * based protocols." [draft-ietf-homenet-dncp-07]
 *
 * This DNCP implementation is not meant to be used stand-alone, but to be used
 * as baseline for specific profiles as e.g. HNCP.
 *
 * @{
 *
 * @file
 * @brief       API for the Distributed Node Consensus Protocol (DNCP)
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_DNCP_H
#define NET_DNCP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "net/dncp/tlvs.h"
#include "ng_nettype.h"
#include "net/ng_ipv6.h"
#include "trickle.h"

#define HNCP_PORT 8808
#define HNCP_DTLS_SERVER_PORT 8809
#define HNCP_MCAST_GROUP "ff02::8808"

#define TRICKLE_K 1
#define TRICKLE_I_MIN 200
#define TRICKLE_I_MAX 7

/**
 * @brief	dncp profile structure
 *
 */
typedef struct {
	ng_nettype_t transport; /**< transport protocol */
	uint16_t port;			/**< port of the transport protocol */
	ng_ipv6_addr_t mcast;	/**< multicast group */
	trickle_t trickle;		/**< trickle parameters */
	void (*cb_hash)(uint8_t *hash, const uint8_t *data, size_t len); /**< hash callback */
	uint8_t node_id_len;	/**< node identified length */
	uint16_t keep_alive_interval;
	float keep_alive_multiplier;
} dncp_profile_t;

/**
 * @brief   Initialization of DNCP
 *
 * @param[in] profile   profile struct
 */
void dncp_init(dncp_profile_t *profile);

/**
 * @brief   Dispatch an incoming TLV block
 *
 * @param[in] tlv       TLV data
 */
void dncp_dispatch(dncp_tlv_t *tlv);

#ifdef __cplusplus
}
#endif

#endif /* NET_DNCP_H */
/** @} */
