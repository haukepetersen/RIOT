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

#include "assert.h"

#include "net/gorm/gap.h"
#include "net/gorm/addr.h"
#include "net/gorm/util.h"
#include "net/gorm/config.h"

typedef struct {
    uint16_t min_interval;
    uint16_t max_interval;
    uint16_t latency;
    uint16_t timeout;
} con_params_t;

static struct {
    const char *name;
    uint16_t appearance;
    con_params_t con_params;
    gorm_gap_adv_ctx_t data;
} config;

void gorm_gap_init(const uint8_t *addr, const char *name, uint16_t appearance)
{
    if (addr) {
        memcpy(config.data.addr, addr, BLE_ADDR_LEN);
    }
    else {
        /* generate random public address */
        gorm_addr_gen_random(config.data.addr);
    }
    config.name = name;
    config.appearance = appearance;
    config.data.interval = GORM_CFG_DEFAULT_ADV_INTERVAL;
    config.con_params.min_interval = GORM_CFG_CON_PARAM_MIN_INTERVAL;
    config.con_params.max_interval = GORM_CFG_CON_PARAM_MAX_INTERVAL;
    config.con_params.latency = GORM_CFG_CON_PARAM_LATENCY;
    config.con_params.timeout = GORM_CFG_CON_PARAM_TIMEOUT;

    /* generate default scan data PDU */
    config.data.adv_len = gorm_gap_ad_add_flags(config.data.adv_data, 0,
                                                GORM_GAP_FLAGS_DEFAULT);
    config.data.adv_len = gorm_gap_ad_add(config.data.adv_data,
                                          config.data.adv_len, BLE_GAP_NAME,
                                          config.name, strlen(config.name));

    /* per default, we do not send any payload on SCAN_REQUESTS */
    config.data.scan_len = 0;
}

size_t gorm_gap_copy_name(uint8_t *buf, size_t max_len)
{
    size_t len = strlen(config.name);
    len = (len > max_len) ? max_len : len;
    memcpy(buf, config.name, len);
    return len;
}

size_t gorm_gap_copy_appearance(uint8_t *buf, size_t max_len)
{
    if (max_len >= 2) {
        gorm_util_htoles(buf, config.appearance);
        return 2;
    }
    return 0;
}

size_t gorm_gap_copy_con_params(uint8_t *buf, size_t max_len)
{
    if (max_len >= 8) {
        gorm_util_htoles(buf, config.con_params.min_interval);
        gorm_util_htoles(buf + 2, config.con_params.max_interval);
        gorm_util_htoles(buf + 4, config.con_params.latency);
        gorm_util_htoles(buf + 6, config.con_params.timeout);
        return 8;
    }
    return 0;
}

gorm_gap_adv_ctx_t *gorm_gap_get_default_adv_ctx(void)
{
    return &config.data;
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

void gorm_gap_print(void)
{
    puts("Gorm's GAP configuration:");
    printf("device name : %s\n", config.name);
    printf("appearance  : 0x%04x\n", (int)config.appearance);
    printf("adv interval: %u\n", (unsigned)config.data.interval);
    puts("preferred connection parameters:");
    printf("  min interval: %uus\n",
           (unsigned)(config.con_params.min_interval * 1250));
    printf("  max interval: %uus\n",
           (unsigned)(config.con_params.max_interval * 1250));
    printf("  latency     : %u\n", (unsigned)config.con_params.latency);
    printf("  timeout     : %uus\n",
           (unsigned)(config.con_params.timeout * 1250));
}
