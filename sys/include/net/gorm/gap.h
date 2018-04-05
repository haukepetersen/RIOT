/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_gap GAP for Gorm
 * @ingroup     net_gorm
 * @brief       Gorm's GAP implementation
 *
 * # Design Decisions and Limitations
 * - no support for sending additional information in a SCAN_RESP packet
 *
 * # Implementation State
 *
 * @{
 *
 * @file
 * @brief       Gorm's GAP mapping
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_GAP_H
#define GORM_GAP_H

#include <stddef.h>
#include <stdint.h>

#include "net/ble.h"
#include "net/gorm/ll.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Maximum AD data length allowed [in bytes]
 */
#define GORM_GAP_AD_MAXLEN          (31U)

/* TODO: move to ble.h */
/**
 * @name    GAP advertisement data type values
 *
 * @see https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
 *
 * @{
 */
#define GORM_GAP_FLAGS              (0x01)
#define GORM_GAP_UUID16_INCOMP      (0x02)
#define GORM_GAP_UUID16_COMP        (0x03)
#define GORM_GAP_UUID32_INCOMP      (0x04)
#define GORM_GAP_UUID32_COMP        (0x05)
#define GORM_GAP_UUID128_INCOMP     (0x06)
#define GORM_GAP_UUID128_COMP       (0x07)
#define GORM_GAP_NAME_SHORT         (0x08)
#define GORM_GAP_NAME               (0x09)
// #define GORM_GAP Tx Power Level (0x0A)
// #define GORM_GAP Class of Device (0x0D)
// #define GORM_GAP Simple Pairing Hash C (0x0E)
// #define GORM_GAP Simple Pairing Hash C-192 (0x0E)
// #define GORM_GAP Simple Pairing Randomizer R (0x0F)
// #define GORM_GAP Simple Pairing Randomizer R-192 (0x0F)
// #define GORM_GAP Device ID (0x10)
// #define GORM_GAP Security Manager TK Value (0x10)
// #define GORM_GAP Security Manager Out of Band Flags (0x11)
// #define GORM_GAP Slave Connection Interval Range (0x12)
// #define GORM_GAP List of 16-bit Service Solicitation UUIDs (0x14)
// #define GORM_GAP List of 128-bit Service Solicitation UUIDs (0x15)
#define GORM_GAP_SERVICE_DATA           (0x16)
#define GORM_GAP_SERVICE_DATA_UUID16    (0x16)
#define GORM_GAP_ADDR_PUBLIC            (0x17)
#define GORM_GAP_ADDR_RANDOM            (0x18)
#define GORM_GAP_APPEARANCE             (0x19)
// #define GORM_GAP Advertising Interval (0x1A)
// #define GORM_GAP LE Bluetooth Device Address (0x1B)
// #define GORM_GAP LE Role (0x1C)
// #define GORM_GAP Simple Pairing Hash C-256 (0x1D)
// #define GORM_GAP Simple Pairing Randomizer R-256 (0x1E)
// #define GORM_GAP List of 32-bit Service Solicitation UUIDs (0x1F)
// #define GORM_GAP Service Data - 32-bit UUID (0x20)
// #define GORM_GAP Service Data - 128-bit UUID (0x21)
// #define GORM_GAP LE Secure Connections Confirmation Value (0x22)
// #define GORM_GAP LE Secure Connections Random Value (0x23)
// #define GORM_GAP URI (0x24)
// #define GORM_GAP Indoor Positioning (0x25)
// #define GORM_GAP Transport Discovery Data (0x26)
// #define GORM_GAP LE Supported Features (0x27)
// #define GORM_GAP Channel Map Update Indication (0x28)
// #define GORM_GAP 3D Information Data (0x3D)
#define GORM_GAP_VENDOR                 (0xFF)
/** @} */

/* TODO: move to ble.h */
/**
 * @name    Available flags in the GORM_GAP_FLAGS field
 * @{
 */
#define GORM_GAP_DISCOVER_LIM           (0x01)
#define GORM_GAP_DISCOVERABLE           (0x02)
#define GORM_GAP_FLAG_BREDR_NOTSUP      (0x04)
/** @} */


#define GORM_GAP_FLAGS_DEFAULT          (GORM_GAP_DISCOVERABLE | \
                                         GORM_GAP_FLAG_BREDR_NOTSUP)


void gorm_gap_init(const char *name, uint16_t appearance);

size_t gorm_gap_copy_name(uint8_t *buf, size_t max_len);

void gorm_gap_copy_appearance(uint8_t *buf);


/**
 * @brief   Add advertising data field to the given buffer
 *
 * @param[out] buf [description]
 * @param[in]  pos
 * @param[in]  type [description]
 * @param[in]  data [description]
 * @param[in]  len [description]
 *
 * @return  overall length of AD data in the buffer
 */
size_t gorm_gap_ad_add(uint8_t *buf, size_t pos,
                        uint8_t type, const void *data, size_t len);


/**
 * @brief   Convenience function for writing flags to given AD data
 *
 * @param[out] buf      AD data buffer
 * @param[in]  flags    flags to use, typically GORM_GAP_FLAGS_DEFAULT
 *
 * @return  always returns 3 (as the flag field is 3 bytes long)
 */
static inline size_t gorm_gap_ad_add_flags(uint8_t *buf, size_t pos,
                                           uint8_t flags)
{
    return gorm_gap_ad_add(buf, pos, GORM_GAP_FLAGS, &flags, 1);
}

/* @todo add functions for reading AD data when running a scanner */

#ifdef __cplusplus
}
#endif

#endif /* GORM_LL_H */
/** @} */
