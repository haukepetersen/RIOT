/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
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
 * @brief       Gorm's link layer implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <string.h>

#include "log.h"

#include "net/ble.h"
#include "net/gorm/ll.h"
#include "net/gorm/ll/trx.h"
#include "net/gorm/arch/rand.h"

// REMOVE
#include "board.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

/* TODO: this address should of course be generated on demand! */
/* TODO: move address handling to its own sub-module */
static uint8_t addr_rand[BLE_ADDR_LEN] = { 0x56, 0x34, 0x12, 0xef, 0xcd, 0xab };

event_queue_t gorm_ll_runqueue;

void gorm_ll_run(netdev_t *dev)
{
    DEBUG("[gorm_ll_run] initializing link layer now\n");

    // LED0_ON;
    // LED1_ON;
    // LED2_ON;
    // LED3_ON;
    LED4_ON;
    LED5_ON;
    LED6_ON;
    LED7_ON;

    /* prepare the event queue, do this first in case events are spawned during
     * the next steps of initialization */
    event_queue_init(&gorm_ll_runqueue);

    /* setup the radio */
    if (gorm_ll_trx_init(dev) != 0) {
        LOG_ERROR("[gorm_ll] error while initializing the radio\n");
        return;
    }

#ifdef MODULE_GORM_PERIPHERAL
    /* setup peripheral role if selected */
    gorm_ll_periph_init();
#endif

    DEBUG("[gorm_ll] starting event loop\n");
    event_loop(&gorm_ll_runqueue);
}


uint8_t *gorm_ll_addr_rand(void)
{
    return addr_rand;
}


#ifdef MODULE_GORM_BROADCASTER

static void on_adv_nonconn(void *arg)
{
    gorm_ll_adv_nonconn_t *adv = (gorm_ll_adv_nonconn_t *)arg;

    /* for now, we hardcode the used advertising channels. This could be
     * something to make configurable in the future, but only maybe... */
    for (uint8_t chan = 37; chan <= 39; chan ++) {
        adv->pkt->chan = chan;
        gorm_ll_send(adv->pkt);
        printf("[gorm_ll] adv_nonconn: send adv pkt on chan %i\n", (int)chan);
    }

    /* add a [0,10ms] `advDelay` to the `advInterval`
     * -> spec v5.0-vol6-b-4.4.2.2.1 */
    uint32_t next = (adv->interval + gorm_arch_rand(0, 10000));
    gorm_arch_evtim_set_from_last(&adv->timer, next);
}

void gorm_ll_adv_nonconn_setup(gorm_ll_adv_nonconn_t *adv,
                               netdev_ble_pkt_t *pkt, uint32_t interval)
{
    assert(adv && pkt && (interval >= 20000) && (interval <= 1485759375));

    /* finish packet initialization (flags, initial CRC, access address) */
    // TODO: move to separate internal function shared with peripheral */
    pkt->aa.u32 = GORM_LL_ADV_AA;
    pkt->hdr.flags = (GORM_LL_ADV_NONCON_IND | GORM_LL_FLAG_TXADD);
    pkt->crc = GORM_LL_ADV_CRC;

    /* configure advertisement context */
    adv->interval = interval;
    adv->pkt = pkt;
    gorm_arch_evtim_init(&adv->timer, on_adv_nonconn, adv);
}

#endif
