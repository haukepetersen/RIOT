/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cwa-tracker
 * @{
 *
 * @file
 * @brief       Corona-Warn-App Tracker
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "fmt.h"
#include "nimble_scanner.h"
#include "net/bluetil/ad.h"
#include "net/bluetil/addr.h"

#include "app.h"

#define ENABLE_DEBUG    0
#include "debug.h"

#define AD_TYPE_SERVICE_LIST    0x03

static const uint8_t _en_uuid[2] = { 0x6f, 0xfd };

static const struct ble_gap_disc_params _scan_params = {
    .itvl = SCANNER_ITVL,
    .window = SCANNER_WIN,
    .filter_policy = 0,
    .limited = 0,
    .passive = 1,                               /* only scan passive: no SCAN_REQ */
    .filter_duplicates = 0,
};

static char _tmp[200];

static scanner_stats_t _stats;

static void _on_disc(uint8_t type,
                     const ble_addr_t *addr, int8_t rssi,
                     const uint8_t *ad, size_t ad_len)
{
    size_t pos = 0;
    uint32_t sec;
    uint32_t ms;

    wallclock_now(&sec, &ms);

    pos += fmt_u32_dec(&_tmp[pos], sec);
    _tmp[pos++] = '.';
    /* write ms with filling zeros */
    size_t len = fmt_u32_dec(NULL, ms);
    for (size_t i = 0; i < (3 - len); i++) {
        _tmp[pos++] = '0';
    }
    pos += fmt_u32_dec(&_tmp[pos], ms);
    _tmp[pos++] = ';';
    pos += fmt_u32_dec(&_tmp[pos], (uint32_t)type);
    _tmp[pos++] = ';';
    pos += fmt_u32_dec(&_tmp[pos], (uint32_t)addr->type);
    _tmp[pos++] = ';';
    bluetil_addr_sprint(&_tmp[pos], addr->val);
    pos += BLUETIL_ADDR_STRLEN - 1;
    _tmp[pos++] = ';';
    pos += snprintf(&_tmp[pos], 4, "%i", (int)rssi);
    // pos += fmt_s16_dec(&_tmp[pos], (int16_t)rssi);
    _tmp[pos++] = ';';
    pos += fmt_bytes_hex(&_tmp[pos], ad, ad_len);
    _tmp[pos++] = '\n';
    _tmp[pos] = '\0';

    DEBUG("%s", _tmp);
    stor_write_ln(_tmp, pos);

    /* see if the advertising data contains the GAEN service */
    bluetil_ad_t adt;
    bluetil_ad_init(&adt, (void *)ad, ad_len, ad_len);
    if (bluetil_ad_find_and_cmp(&adt, AD_TYPE_SERVICE_LIST, _en_uuid, 2)) {
        ++_stats.cwa_cnt;
    }

    ++_stats.pkt_cnt;
}

int scanner_init(void)
{
    int res = nimble_scanner_init(&_scan_params, _on_disc);
    if (res != 0) {
        return res;
    }
    return nimble_scanner_start();
}

void scanner_getcount(scanner_stats_t *stats)
{
    stats->pkt_cnt = _stats.pkt_cnt;
    stats->cwa_cnt = _stats.cwa_cnt;
}
