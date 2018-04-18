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
 * # Default Advertising Data
 * Per default, Gorm nodes advertise the following fields:
 * - Flags (GORM_GAP_FLAGS)
 * - Complete Local Name (GORM_GAP_NAME)
 *
 * On scan requests, the node will per default answer with an empty scan
 * response.
 *
 * To override these settings, use the API functions provided by this module to
 * write custom fields to the adv_data and scan_data buffers.
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

#ifndef NET_GORM_GAP_H
#define NET_GORM_GAP_H

#include <stddef.h>
#include <stdint.h>

#include "net/ble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Maximum AD data length allowed [in bytes]
 */
#define GORM_GAP_AD_MAXLEN          (31U)


#define GORM_GAP_FLAGS_DEFAULT          (BLE_GAP_DISCOVERABLE | \
                                         BLE_GAP_FLAG_BREDR_NOTSUP)


typedef struct {
    uint8_t addr[BLE_ADDR_LEN];
    uint8_t adv_data[GORM_GAP_AD_MAXLEN];
    size_t adv_len;
    uint8_t scan_data[GORM_GAP_AD_MAXLEN];
    size_t scan_len;
    uint32_t interval;
} gorm_gap_adv_ctx_t;

void gorm_gap_init(const uint8_t *addr, const char *name, uint16_t appearance);

void gorm_gap_set_name(const char *name);
void gorm_gap_set_appearance(uint16_t appearance);
void gorm_gap_set_pref_con_params(uint16_t min_interval, uint16_t max_interval,
                                  uint16_t latency, uint16_t timeout);
void gorm_gap_set_addr(const uint8_t *addr);

size_t gorm_gap_copy_name(uint8_t *buf, size_t max_len);
size_t gorm_gap_copy_appearance(uint8_t *buf, size_t max_len);
size_t gorm_gap_copy_con_params(uint8_t *buf, size_t max_len);

gorm_gap_adv_ctx_t *gorm_gap_get_default_adv_ctx(void);

uint8_t *gorm_gap_get_adv_pdu(void);
uint8_t *gorm_gap_get_scan_resp_pdu(void);



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

/**
 * @brief   Dump GAP configuration to STDOUT
 */
void gorm_gap_print(void);

#ifdef __cplusplus
}
#endif

#endif /* NET_GORM_GAP_H */
/** @} */
