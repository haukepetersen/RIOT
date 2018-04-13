/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm
 * @{
 *
 * @file
 * @brief       Gorm's GAP layer implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <string.h>

#include "assert.h"

#include "net/gorm/gap.h"
#include "net/gorm/util.h"

static const char *device_name;
static uint16_t device_appearance;

struct {
    uint16_t min_interval;
    uint16_t max_interval;
    uint16_t latency;
    uint16_t timeout;
} con_params;

struct {
    /* TODO: cater for different addresses (public, randon, private) */
    /* TODO: how to save TxAdd flag? for marking public addresses? */
    uint8_t addr[BLE_ADDR_LEN];
    uint8_t *ad_adv;
    size_t ad_adv_len;
    uint8_t *ad_scan;
    size_t ad_scan_len;
    /* TODO: move this and a pointer to this this struct into con_t */
    uint8_t adv_chan;
} adv_data;

void gorm_gap_init(const char *name, uint16_t appearance)
{
    device_name = name;
    device_appearance = appearance;
}

size_t gorm_gap_copy_name(uint8_t *buf, size_t max_len)
{
    size_t len = strlen(device_name);
    len = (len > max_len) ? max_len : len;
    memcpy(buf, device_name, len);
    return len;
}

void gorm_gap_copy_appearance(uint8_t *buf)
{
    gorm_util_htoles(buf, device_appearance);
}

size_t gorm_gap_ad_add(uint8_t *buf, size_t pos,
                       uint8_t type, const void *data, size_t len)
{
    assert(buf && ((pos + 2 + len) <= GORM_GAP_AD_MAXLEN));

    uint8_t *ptr = buf + pos;
    *ptr++ = (uint8_t)(len + 1);
    *ptr++ = type;
    memcpy(ptr, data, len);

    return (pos + 2 + len);
}
