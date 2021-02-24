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
#include <stdio.h>
#include <errno.h>

#include "assert.h"

#include "net/gorm.h"
#include "net/gorm/ll.h"
#include "net/gorm/gap.h"
#include "net/gorm/addr.h"
#include "net/gorm/util.h"
#include "net/gorm/config.h"

int gorm_gap_adv_init(gorm_gap_adv_ctx_t *ctx, uint32_t interval_ms,
                      uint8_t *ad, size_t ad_len,
                      uint8_t *sd, size_t sd_len)
{
    // TODO remove magic numbers
    if ((interval_ms < 20) || (interval_ms > 10485)) {
        return -EINVAL;
    }
    if ((ad == NULL) || (ad_len == 0)) {
        return -ENODATA;
    }
    if ((ad_len > BLE_ADV_PDU_LEN) || (sd_len > BLE_ADV_PDU_LEN)) {
        return -ENOBUFS;
    }

    ctx->interval_ms = interval_ms;
    memcpy(ctx->ad, ad, ad_len);
    ctx->ad_len = ad_len;
    memcpy(ctx->sd, sd, sd_len);
    ctx->sd_len = sd_len;

    return 0;
}

int gorm_gap_conn_init(gorm_gap_conn_ctx_t *ctx, uint16_t interval_ms,
                                 uint16_t timeout_ms, uint16_t latency)
{
    // TODO remove magic numbers
    // TODO: check precise timeout and latency
    if ((interval_ms < 8) || (interval_ms > 4000) ||
        (timeout_ms < 100) || (timeout_ms > 32000) ||
        (latency > 500)) {
        return -EINVAL;
    }

    ctx->interval_ms = interval_ms;
    ctx->timeout_ms = timeout_ms;
    ctx->latency = latency;
    return 0;
}

int gorm_gap_scan_init(gorm_gap_scan_ctx_t *ctx,
                       uint32_t timeout_ms,
                       uint16_t interval_ms, uint16_t window_ms)
{
    // TODO remove magic numbers
    if ((window_ms > interval_ms) || (interval_ms > 40960)) {
        return -EINVAL;
    }

    ctx->timeout_ms = timeout_ms;
    ctx->interval_ms = interval_ms;
    ctx->window_ms = window_ms;
    return 0;
}

int gorm_gap_adv_start(gorm_ctx_t *con, gorm_gap_adv_ctx_t *adv_ctx,
                       gorm_gap_adv_mode_t mode)
{
    assert(con);
    assert(adv_ctx);

    /* TODO: address should be set higher up I guess */
    memcpy(adv_ctx->addr, gorm_addr_get_own(), BLE_ADDR_LEN);
    return gorm_ll_adv(&con->ll, adv_ctx, mode);
}

int gorm_gap_adv_stop(gorm_ctx_t *con);



// typedef struct {
//     uint16_t min_interval;
//     uint16_t max_interval;
//     uint16_t latency;
//     uint16_t timeout;
// } con_params_t;

// static struct {
//     const char *name;
//     uint16_t appearance;
//     con_params_t con_params;
//     // gorm_gap_adv_ctx_t data;
// } config;

// void gorm_gap_init(const uint8_t *addr, const char *name, uint16_t appearance)
// {
//     if (addr) {
//         memcpy(config.data.addr, addr, BLE_ADDR_LEN);
//     }
//     else {
//         /* generate random public address */
//         gorm_addr_gen_random(config.data.addr);
//     }
//     config.name = name;
//     config.appearance = appearance;
//     config.data.interval = GORM_CFG_DEFAULT_ADV_INTERVAL;
//     config.con_params.min_interval = GORM_CFG_CON_PARAM_MIN_INTERVAL;
//     config.con_params.max_interval = GORM_CFG_CON_PARAM_MAX_INTERVAL;
//     config.con_params.latency = GORM_CFG_CON_PARAM_LATENCY;
//     config.con_params.timeout = GORM_CFG_CON_PARAM_TIMEOUT;

//     /* generate default scan data PDU */
//     config.data.adv_len = gorm_gap_ad_add_flags(config.data.adv_data, 0,
//                                                 BLUETIL_AD_FLAGS_DEFAULT);
//     config.data.adv_len = gorm_gap_ad_add(config.data.adv_data,
//                                           config.data.adv_len, BLE_GAP_AD_NAME,
//                                           config.name, strlen(config.name));

//     /* per default, we do not send any payload on SCAN_REQUESTS */
//     config.data.scan_len = 0;
// }

// size_t gorm_gap_copy_name(uint8_t *buf, size_t max_len)
// {
//     size_t len = strlen(config.name);
//     len = (len > max_len) ? max_len : len;
//     memcpy(buf, config.name, len);
//     return len;
// }

// size_t gorm_gap_copy_appearance(uint8_t *buf, size_t max_len)
// {
//     if (max_len >= 2) {
//         gorm_util_htoles(buf, config.appearance);
//         return 2;
//     }
//     return 0;
// }

// size_t gorm_gap_copy_con_params(uint8_t *buf, size_t max_len)
// {
//     if (max_len >= 8) {
//         gorm_util_htoles(buf, config.con_params.min_interval);
//         gorm_util_htoles(buf + 2, config.con_params.max_interval);
//         gorm_util_htoles(buf + 4, config.con_params.latency);
//         gorm_util_htoles(buf + 6, config.con_params.timeout);
//         return 8;
//     }
//     return 0;
// }

// gorm_gap_adv_ctx_t *gorm_gap_get_default_adv_ctx(void)
// {
//     return &config.data;
// }

// size_t gorm_gap_ad_add(uint8_t *buf, size_t pos,
//                        uint8_t type, const void *data, size_t len)
// {
//     assert(buf && ((pos + 2 + len) <= BLE_ADV_PDU_LEN));

//     uint8_t *ptr = buf + pos;
//     *ptr++ = (uint8_t)(len + 1);
//     *ptr++ = type;
//     memcpy(ptr, data, len);

//     return (pos + 2 + len);
// }

// void gorm_gap_print(void)
// {
//     puts("Gorm's GAP configuration:");
//     printf("device name : %s\n", config.name);
//     printf("appearance  : 0x%04x\n", (int)config.appearance);
//     printf("adv interval: %u\n", (unsigned)config.data.interval);
//     puts("preferred connection parameters:");
//     printf("  min interval: %uus\n",
//            (unsigned)(config.con_params.min_interval * 1250));
//     printf("  max interval: %uus\n",
//            (unsigned)(config.con_params.max_interval * 1250));
//     printf("  latency     : %u\n", (unsigned)config.con_params.latency);
//     printf("  timeout     : %uus\n",
//            (unsigned)(config.con_params.timeout * 1250));
// }
