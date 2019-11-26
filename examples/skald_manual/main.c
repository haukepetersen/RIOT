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
 * @brief       BLE Eddystone beacon example using Skald
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "log.h"

#include "random.h"
#include "net/skald.h"
#include "periph/rtt.h"
#include "net/bluetil/ad.h"

#ifndef NODE_NAME
#define NODE_NAME           "Skaldman"
#endif

#ifndef ADV_ITVL
#define ADV_ITVL            RTT_SEC_TO_TICKS(1)
#endif

#define JITTER_MIN          (ADV_ITVL)
#define JITTER_MAX          (ADV_ITVL + RTT_MS_TO_TICKS(10)) /* +10 ms */

static skald_ctx_t _ctx;

static void _adv_evt(void *arg)
{
    (void)arg;
    skald_adv_trigger(&_ctx);
    rtt_set_alarm(random_uint32_range(JITTER_MIN, JITTER_MAX), _adv_evt, NULL);
}

int main(void)
{
    LOG_INFO("Skald and the tail of Eddystone\n");

    int res;
    (void)res;
    bluetil_ad_t ad;
    uint8_t *pdu = _ctx.pkt.pdu;

    skald_generate_random_addr(pdu);
    res = bluetil_ad_init_with_flags(&ad, (_ctx.pkt.pdu + 6),
                                     (sizeof(_ctx.pkt.pdu) - 6),
                                     BLE_GAP_FLAG_BREDR_NOTSUP);
    assert(res == BLUETIL_AD_OK);
    res = bluetil_ad_add_name(&ad, NODE_NAME);

    /* trigger initial advertising event */
    _adv_evt(NULL);

    return 0;
}
