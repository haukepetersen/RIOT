/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
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
#include "net/bluetil/ad.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef enum {
    GORM_GAP_CONN_NON = 0x00,
    GORM_GAP_CONN_UND = 0x01,
    GORM_GAP_CONN_DIR = 0x02,
} gorm_gap_adv_mode_t;

typedef struct gorm_ctx gorm_ctx_t;

/**
 * @brief   Structure holding an advertising context
 */
typedef struct {
    uint16_t interval_ms;                   /**< advertising interval [in ms] */
    uint8_t addr[BLE_ADDR_LEN];             /**< device address */
    uint8_t ad[BLE_ADV_PDU_LEN];      /**< advertising payload */
    size_t ad_len;                         /**< size of adv_data */
    uint8_t sd[BLE_ADV_PDU_LEN];     /**< SCAN_RESPONSE payload */
    size_t sd_len;                        /**< size of scan_len */
} gorm_gap_adv_ctx_t;

typedef struct {
    uint16_t interval_ms;
    uint16_t timeout_ms;
    uint16_t latency;
} gorm_gap_conn_ctx_t;

typedef struct {
    uint32_t timeout_ms;
    uint16_t interval_ms;
    uint16_t window_ms;
} gorm_gap_scan_ctx_t;

/**
 * @brief   Initialize the GAP module
 *
 * @param[in] addr          device address
 * @param[in] name          device name, as '\0' terminated string
 * @param[in] appearance    custom device appearance
 */
// void gorm_gap_init(const uint8_t *addr, const char *name, uint16_t appearance);

int gorm_gap_adv_init(gorm_gap_adv_ctx_t *ctx, uint32_t interval_ms,
                      uint8_t *ad, size_t ad_len,
                      uint8_t *sd, size_t sd_len);

int gorm_gap_conn_init(gorm_gap_conn_ctx_t *ctx, uint16_t interval_ms,
                                 uint16_t timeout_ms, uint16_t latency);

int gorm_gap_scan_init(gorm_gap_scan_ctx_t *ctx,
                       uint32_t timeout_ms,
                       uint16_t interval_ms, uint16_t window_ms);

int gorm_gap_adv_start(gorm_ctx_t *con, gorm_gap_adv_ctx_t *adv_ctx,
                       gorm_gap_adv_mode_t mode);

int gorm_gap_adv_stop(gorm_ctx_t *con);

/**
 * @brief   Change the advertised device address
 *
 * @param[in] addr          new device address, must hold 6 bytes
 */
// void gorm_gap_set_addr(const uint8_t *addr);

/**
 * @brief   Change the advertised device name
 *
 * @param[in] name          new device name
 */
// void gorm_gap_set_name(const char *name);

/**
 * @brief   Change the advertised device's appearance
 *
 * @param[in] appearance    new appearance
 */
// void gorm_gap_set_appearance(uint16_t appearance);

/**
 * @brief    Change the device's preferred connection parameters
 *
 * The initial configuration defined in Gorm's main configuration file
 * (`net/gorm/config.h`).
 *
 * @param[in] min_interval  preferred minimum connection interval
 * @param[in] max_interval  preferred maximum connection interval
 * @param[in] latency       preferred slave latency
 * @param[in] timeout       preferred connection timeout
 */
// void gorm_gap_set_pref_con_params(uint16_t min_interval, uint16_t max_interval,
//                                   uint16_t latency, uint16_t timeout);

/**
 * @brief   Copy the device name into the given buffer
 *
 * @param[out] buf          buffer to write to
 * @param[in]  max_len      maximum number of bytes to write to @p buf
 *
 * @return  number of bytes written to @p buf
 */
// size_t gorm_gap_copy_name(uint8_t *buf, size_t max_len);

/**
 * @brief   Write the device's appearance into the given buffer
 *
 * @param[out] buf          buffer to write to
 * @param[in]  max_len      size of @p buf in bytes, will only write if >= 2
 *
 * @return   2 if appearance was written successfully
 * @return   0 if not enough space in @p buf
 */
// size_t gorm_gap_copy_appearance(uint8_t *buf, size_t max_len);

/**
 * @brief   Write the preferred connection parameters to the given buffer
 *
 * @param[out] buf          write preferred connection parameters to this buffer
 * @param[in]  max_len      size of @p buf in bytes, will only write if >= 8
 *
 * @return  8 if preferred connection parameters were written to @p buf
 * @return  0 if not enough space in @p buf
 */
// size_t gorm_gap_copy_con_params(uint8_t *buf, size_t max_len);

/**
 * @brief   Get the default advertising context
 *
 * @return  pointer to default advertising context
 */
// gorm_gap_adv_ctx_t *gorm_gap_get_default_adv_ctx(void);

/**
 * @brief   Add advertising data field to the given buffer
 *
 * @todo    remove in favor of bluetil/ad
 *
 * @param[out] buf          buffer to write to
 * @param[in]  pos          position in @p buf to write to
 * @param[in]  type         advertising data type
 * @param[in]  data         advertising data field payload
 * @param[in]  len          size of @p data in bytes
 *
 * @return  overall length of AD data in the buffer
 */
// size_t gorm_gap_ad_add(uint8_t *buf, size_t pos,
//                        uint8_t type, const void *data, size_t len);

/**
 * @brief   Convenience function for writing flags to given AD data
 *
 * @todo    remove in favor of bluetil/ad
 *
 * @param[out] buf          AD data buffer
 * @param[in]  pos          position in @p buf to write to
 * @param[in]  flags        flags to use, typically BLUETIL_AD_FLAGS_DEFAULT
 *
 * @return  always returns 3 (as the flag field is 3 bytes long)
 */
// static inline size_t gorm_gap_ad_add_flags(uint8_t *buf, size_t pos,
//                                            uint8_t flags)
// {
//     return gorm_gap_ad_add(buf, pos, BLUETIL_AD_FLAGS_DEFAULT, &flags, 1);
// }

/**
 * @brief   Dump GAP configuration to STDOUT
 *
 * @todo    ifdef somehow?!
 */
// void gorm_gap_print(void);

#ifdef __cplusplus
}
#endif

#endif /* NET_GORM_GAP_H */
/** @} */
