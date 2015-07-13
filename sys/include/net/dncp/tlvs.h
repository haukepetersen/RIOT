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
 * @brief       TLV definitions for the DNCP protocol
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_DNCP_TLVS_H
#define NET_DNCP_TLVS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Global DNCP TLV type definitions
 * @{
 */
#define DNCP_TLV_TYPE_RESERVED              (0U)
#define DNCP_TLV_TYPE_REQ_NET_STATE         (1U)
#define DNCP_TLV_TYPE_REQ_NODE_STATE        (2U)
#define DNCP_TLV_TYPE_NODE_EP               (3U)
#define DNCP_TLV_TYPE_NET_STATE             (4U)
#define DNCP_TLV_TYPE_NODE_STATE            (5U)
#define DNCP_TLV_TYPE_FRAG_COUNT            (7U)
#define DNCP_TLV_TYPE_NEIGHBOR              (8U)
#define DNCP_TLV_TYPE_KEEP_ALIVE_INT        (9U)
#define DNCP_TLV_TYPE_TRUST_VERDICT         (10U)
/**  @} */

/**
 * @brief   Generic TLV data structure
 *
 * This is send over the air, so it needs to be packed!
 */
typedef struct __attribute__((packed)) {
    network_uint16_t type;      /**< 16-bit type field, network byte order */
    network_uint16_t length;    /**< 16-bit length field, network byte order */
    uint8_t value[];            /**< variable length data */
} dncp_tlv_t;

#ifdef __cplusplus
}
#endif

#endif /* NET_DNCP_TLVS_H */
/** @} */
