/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_ipv4
 * @{
 *
 * @file
 * @brief       Definitions for IPv4 addresses
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef IPV4_ADDR_H
#define IPV4_ADDR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   IPv4 address length in byte
 */
#define IPV4_ADDR_LEN               (4U)

/**
 * @brief   IPv4 address length in bit
 */
#define IPV4_ADDR_BIT_LEN           (32U)

/**
 * @brief   Maximum length of an IPv4 address as string
 */
#define IPV4_ADDR_MAX_STR_LEN       (15U)

/**
 * @brief   Representation of IPv4 addresses
 */
typedef union {
    uint8_t u8[4];              /**< divided by 4 8-bit words */
    network_uint16_t u16[2];    /**< divided by 2 16-bit words */
    network_uint32_t u32;       /**< as 32-bit word */
} ipv4_addr_t;

/**
 * @brief   Default loopback address
 */
#define IPV4_ADDR_LOOPBACK          {{ 0x7f, 0, 0, 1}}

#ifdef __cplusplus
}
#endif

#endif /* IPV4_ADDR_H */
/** @} */
