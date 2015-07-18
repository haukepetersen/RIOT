/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_hncp
 * @{
 *
 * @file
 * @brief       TLV definitions for the HNCP protocol
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_HNCP_TLVS_H
#define NET_HNCP_TLVS_H

#include <stdio.h>

#include "net/ng_ipv6/addr.h"
#include "net/ipv4/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   HNCP specific TLV type definitions
 * @{
 */
#define HNCP_TLV_TYPE_EXT_CON           (33U)
#define HNCP_TLV_TYPE_DEL_PREFIX        (34U)
#define HNCP_TLV_TYPE_ASS_PREFIX        (35U)
#define HNCP_TLV_TYPE_NODE_ADDR         (36U)
#define HNCP_TLV_TYPE_DHCPV4_DATA       (37U)
#define HNCP_TLV_TYPE_DHCPV6_DATA       (38U)
#define HNCP_TLV_TYPE_DNS_DEL_ZONE      (39U)
#define HNCP_TLV_TYPE_DOMAIN_NAME       (40U)
#define HNCP_TLV_TYPE_NODE_NAME         (41U)
#define HNCP_TLV_TYPE_MANAGED_PSK       (42U)
/**  @} */

/**
 * @brief   Node address TLV definition
 */
typedef struct __attribute__((packed)) {
    network_uint16_t type;          /**< TLV type, must be 36*/
    network_uint16_t length;        /**< length, must be 20 */
    network_uint32_t ep_id;         /**< endpoint identifier */
    union {                    /**< IP address (IPv4 or IPv6) */
        struct {               /**< IPv4 mapped IPv6 address [RFC 4291] */
            uint8_t reserved0[80];  /**< must be all zeros */
            uint8_t reserved1[16];  /**< must be all ones */
            ipv4_addr_t addr;       /**< the actual IPv4 address */
        } ipv4;
        ng_ipv6_addr_t ipv6;        /**< IPv6 address */
    } addr;
} hncp_tlv_node_addr_t;

#ifdef __cplusplus
}
#endif

#endif /* NET_HNCP_TLVS_H */
/** @} */
