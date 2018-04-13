/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       BLE GATT peripheral example demonstrating Gorm's GATT/GAP usage
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "net/gorm/ll.h"
#include "net/gorm/gap.h"
#include "net/gorm/host.h"
#include "net/gorm/gatt.h"
#include "net/gorm/gatt/tab.h"
#include "net/gorm/gatt/desc.h"
#include "net/ggs/riot.h"

#define UUID_INFO       (0x0815)
#define UUID_INFO_TEXT  (0x0816)

static uint8_t gap_ad[GORM_GAP_AD_MAXLEN];

// static gorm_ctx_t _con;

static const char *device_name = "RIOT";
static const char *info_str = "Hackschnitzel";

static size_t _info_cb(const gorm_gatt_char_t *characteristic, uint8_t method,
                       uint8_t *buf, size_t buf_len);

static gorm_gatt_entry_t info_entry;

static const gorm_gatt_desc_t info_desc[] = {
    GORM_GATT_DESC_FMT(BLE_UNIT_BLE_FMT_UTF8, 0, BLE_UNIT_NONE),
    GORM_GATT_DESC_EXT_PROP(0, 0),
};

static const gorm_gatt_char_t info_char[] = {
    {
        .type     = GORM_UUID(UUID_INFO_TEXT, &ggs_uuid_riot_base),
        .arg      = (void *)23,
        .perm     = (BLE_ATT_READ | BLE_ATT_WRITE),
        .cb       = _info_cb,
        .desc_cnt = 1,
        .desc     = info_desc,
    },
};

static const gorm_gatt_service_t info_service = {
    .uuid     = GORM_UUID(UUID_INFO, &ggs_uuid_riot_base),
    .char_cnt = 1,
    .chars    = info_char,
};

static size_t _info_cb(const gorm_gatt_char_t *characteristic, uint8_t method,
                      uint8_t *buf, size_t buf_len)
{
    (void)characteristic;

    if (method == GORM_GATT_READ) {
        printf("READ READ READ: _info_cb()\n");
        size_t size = strlen(info_str);
        size = (size > buf_len) ? buf_len : size;
        memcpy(buf, info_str, size);
        return size;
    }
    else if (method == GORM_GATT_WRITE) {
        printf("WRITE WRITE WRITE: _info_cb():");
        for (size_t i = 0; i < buf_len; i++) {
            printf("%c", (char)buf[i]);
        }
        puts("");
        return GORM_GATT_OP_OK;
    }

    return GORM_GATT_OP_NOT_SUPPORTED;
}

int main(void)
{
    puts("Gorm's GATT example");

    /* register the GATT service */
    gorm_gatt_tab_reg_service(&info_entry, &info_service);

    /* setup the GAP fields for advertising our services */
    gorm_gap_init(device_name, 0);
    /* TODO: move this to some more central place away from the user */
    /* TODO: make this run per default but enable custom way to do it */
    size_t pos = gorm_gap_ad_add_flags(gap_ad, 0, GORM_GAP_FLAGS_DEFAULT);
    pos = gorm_gap_ad_add(gap_ad, pos, GORM_GAP_NAME, device_name, 4);
    /* TODO: remove and make implicit by connection host and GAP modules */
    gorm_ll_periph_adv_setup(gap_ad, pos, NULL, 0);

    /* start advertising this node */
    // gorm_gap_adv_start();

    return 0;
}
