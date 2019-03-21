/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_bluetil_addr BLE address helper
 * @ingroup     net
 * @brief       Generic BLE address handling functions
 * @{
 *
 * @file
 * @brief       Interface for the generic BLE address helper functions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_BLUETIL_ADDR_H
#define NET_BLUETIL_ADDR_H

#include <stdint.h>

#include "net/ble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The length of a BLE address string in bytes
 */
#define BLUETIL_ADDR_STRLEN         (18U)

/**
 * @brief   Convert the given BLE address to a human readable string
 *
 * @param[out] out      '\0' terminated address string, *must* be able to hold
 *                      BLUETIL_ADDR_STRLEN bytes
 * @param[in] addr      address buffer, is expected to hold BLE_ADDR_LEN bytes
 */
void bluetil_addr_to_str(char *out, const uint8_t *addr);

/**
 * @brief   Print the given BLE address to STDOUT
 *
 * @param[in] addr      address to print, is expected to hold BLE_ADDR_LEN bytes
 */
void bluetil_addr_print(const uint8_t *addr);

/**
 * @brief   Print the IPv6 IID generated for the given BLE address
 *
 * @param[in] addr      generate IID for this address
 */
void bluetil_addr_print_ipv6_iid(const uint8_t *addr);

#ifdef __cplusplus
}
#endif

#endif /* NET_BLUETIL_ADDR_H */
/** @} */
