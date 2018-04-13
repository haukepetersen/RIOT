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
 * @brief       Gorm's channel selection algorithm implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "net/ble.h"
#include "net/gorm/ll/chan.h"

static int _chan_used(const uint8_t *map, uint8_t num)
{
    return (map[num >> 3] & (1 << (num & 0x07)));
}

uint8_t gorm_ll_chan_count(const uint8_t *map)
{
    uint8_t cnt = 0;
    for (uint8_t i = 0; i < BLE_CHAN_NUMOF; i++) {
        if (_chan_used(map, i)) {
            ++cnt;
        }
    }
    return cnt;
}

void gorm_ll_chan_algo1(gorm_ll_ctx_t *con)
{
    con->chan_unmapped = (con->chan_unmapped + con->chan_hop) %
                         BLE_CHAN_DAT_NUMOF;
    if (_chan_used(con->chan_map, con->chan_unmapped)) {
        con->ctx.chan = con->chan_unmapped;
    }
    else {
        unsigned remap_index = (con->chan_unmapped % con->chan_cnt) + 1;
        int pos = -1;
        while (remap_index > 0) {
            /* advance to the next used channel */
            while (_chan_used(con->chan_map, ++pos) == 0) {}
            --remap_index;
        }
        con->ctx.chan = pos;
    }
}
