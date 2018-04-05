/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm_gatt
 * @{
 *
 * @file
 * @brief       GATT characteristic description formats
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_GATT_DESC_H
#define GORM_GATT_DESC_H

#include <stdint.h>

#include "net/gorm/gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GORM_GATT_DESC_FMT(f, e, u)                     \
    {                                                   \
        .type = BLE_DESC_PRES_FMT,                      \
        .cb   = gorm_gatt_desc_pres_fmt_cb,             \
        .arg  = (uint32_t)((f << 24) | (e << 16) | u),  \
    }

#define GORM_GATT_DESC_EXT_PROP(rwe, wae)                       \
    {                                                           \
        .type = BLE_DESC_EXT_PROP,                              \
        .cb   = gorm_gatt_desc_ext_prop_cb,                     \
        .arg  = (uint32_t)((rwe & 0x1) | ((wae & 0x01) << 1)),  \
    }

#define GORM_GATT_DESC_USER_DESC(str_ptr)               \
    {                                                   \
        .type = BLE_DESC_USER_DESC,                     \
        .cb   = gorm_gatt_desc_user_desc_cb,            \
        .arg  = (uint32_t)str_ptr,                      \
    }

#define GORM_GATT_DESC_CLIENT_CONF(noe, ine)                    \
    {                                                           \
        .type = BLE_DESC_CLIENT_CONFIG,                         \
        .cb   = gorm_gatt_desc_client_conf_cb,                  \
        .arg  = (uint32_t)((noe & 0x01) | ((ine & 0x01) << 1)), \
    }

size_t gorm_gatt_desc_pres_fmt_cb(const gorm_gatt_desc_t *desc,
                                  uint8_t *buf, size_t buf_len);

size_t gorm_gatt_desc_ext_prop_cb(const gorm_gatt_desc_t *desc,
                                  uint8_t *buf, size_t buf_len);

size_t gorm_gatt_desc_user_desc_cb(const gorm_gatt_desc_t *desc,
                                   uint8_t *buf, size_t buf_len);

size_t gorm_gatt_desc_client_conf_cb(const gorm_gatt_desc_t *desc,
                                     uint8_t *buf, size_t buf_len);

#ifdef __cplusplus
}
#endif

#endif /* GORM_GATT_DESC_H */
/** @} */

