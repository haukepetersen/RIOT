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

#include "net/gorm.h"
#include "net/gorm/gap.h"
#include "net/gorm/gatt/tab.h"
#include "net/gorm/gatt/desc.h"
#include "net/ggs/riot.h"

/* for the example, we chose the appearance of a generic computer */
#define APPEARANCE      (0x0080)

#define UUID_INFO       (0x0815)
#define UUID_INFO_TEXT  (0x0816)

static const char *device_name = "RIOT";
static char *info_str = "Hackschnitzel";

static size_t _info_cb(const gorm_gatt_char_t *characteristic, uint8_t method,
                      uint8_t *buf, size_t buf_len)
{
    (void)characteristic;

    size_t size = strlen(info_str);

    if (method == GORM_GATT_READ) {
        size = (size > buf_len) ? buf_len : size;
        memcpy(buf, info_str, size);
    }
    else if (method == GORM_GATT_WRITE) {
        size = (buf_len > size) ? size : buf_len;
        for (size_t i = 0; i < buf_len; i++) {
            info_str[i] = (char)buf[i];
        }
        info_str[size] = '\0';
        printf("info_cb - WRITE: written '%s'\n", info_str);
    }

    return size;
}

static const gorm_gatt_desc_t info_desc[] = {
    GORM_GATT_DESC_FMT(BLE_UNIT_BLE_FMT_UTF8, 0, BLE_UNIT_NONE),
    GORM_GATT_DESC_EXT_PROP(0, 0),
};

static const gorm_gatt_char_t info_char[] = {
    {
        .type     = GORM_UUID(UUID_INFO_TEXT, &ggs_riot_uuid_base),
        .arg      = (void *)23,
        .perm     = (BLE_ATT_READ | BLE_ATT_WRITE),
        .cb       = _info_cb,
        .desc_cnt = 1,
        .desc     = info_desc,
    },
};

static gorm_gatt_service_t info_service = {
    .uuid     = GORM_UUID(UUID_INFO, &ggs_riot_uuid_base),
    .char_cnt = 1,
    .chars    = info_char,
};

int main(void)
{
    puts("Gorm's GATT example\n");

    /* register the GATT service */
    gorm_gatt_tab_reg_service(&info_service);

    /* setup the GAP fields for advertising our services */
    gorm_gap_init(NULL, device_name, APPEARANCE);
    gorm_gap_print();

    /* start advertising */
    gorm_adv_start();

    /* run Gorm's event loop (so Gorm will take over the main thread) */
    gorm_run();

    /* never reached */
    return 0;
}
