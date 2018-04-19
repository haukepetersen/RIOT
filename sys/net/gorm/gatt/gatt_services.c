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
 * @brief       Implementation of mandatory GATT services
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#include "net/gorm/gatt.h"
#include "net/gorm/gatt/tab.h"

static size_t _on_gap_name(const gorm_gatt_char_t *characteristic,
                           uint8_t method, uint8_t *buf, size_t buf_len)
{
    (void)characteristic;

    if (method != GORM_GATT_READ) {
        return 0;
    }
    return gorm_gap_copy_name(buf, buf_len);
}

static size_t _on_gap_appearance(const gorm_gatt_char_t *characteristic,
                                 uint8_t method, uint8_t *buf, size_t buf_len)
{
    (void)characteristic;

    if (method != GORM_GATT_READ) {
        return 0;
    }
    return gorm_gap_copy_appearance(buf, buf_len);
}

static size_t _on_gap_con_param(const gorm_gatt_char_t *characteristic,
                                uint8_t method, uint8_t *buf, size_t buf_len)
{
    (void)characteristic;

    if (method != GORM_GATT_READ) {
        return 0;
    }
    return gorm_gap_copy_con_params(buf, buf_len);
}

/* declare mandatory services (GAP & ATT) */
static gorm_gatt_service_t gap_service = {
    .uuid  = GORM_UUID(GORM_UUID_GAP, NULL),
    .char_cnt = 3,
    .chars = (gorm_gatt_char_t[]){
        {
            .cb   = _on_gap_name,
            .type = GORM_UUID(GORM_UUID_DEVICE_NAME, NULL),
            .perm = BLE_ATT_READ,
        },
        {
            .cb   = _on_gap_appearance,
            .type = GORM_UUID(GORM_UUID_APPEARANCE, NULL),
            .perm = BLE_ATT_READ,
        },
        {
            .cb   = _on_gap_con_param,
            .type = GORM_UUID(GORM_UUID_PREF_CON_PARAM, NULL),
            .perm = BLE_ATT_READ,
        },
    },
};

static gorm_gatt_service_t att_service = {
    .uuid  = GORM_UUID(GORM_UUID_ATT, NULL),
    .char_cnt = 0,
    .chars = NULL,
};


void gorm_gatt_services_init(void)
{
    gorm_gatt_tab_reg_service(&gap_service);
    gorm_gatt_tab_reg_service(&att_service);
}
