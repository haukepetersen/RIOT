/*
 * Copyright (C) 2020 Hauke Petersen <devel@haukepetersen.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     foo
 * @{
 *
 * @file
 * @brief       Blubb
 *
 * @author      Hauke Petersen <devel@haukepetersen.de>
 *
 * @}
 */

/*
 * trigger: x from neg to pos
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "log.h"
#include "ztimer.h"
#include "net/skald.h"
#include "periph/adc.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

static skald_ctx_t _adv;

#define BAT_MUL             (14652)
#define BAT_DIV             (10000)


int main(void)
{
    uint8_t buf[] = { 0x03, 0x03, 0x03 };

    adc_init(NRF52_VDDHDIV5);

    /* setup BLE */
    uint8_t ad_base[] = { 0x02, 0x01, 0x06,
                          (3 + 4), 0xff, 0xfe, 0xaf, 0xff };

    uint8_t *pdu = _adv.pkt.pdu;
    _adv.pkt.len = (6 + sizeof(ad_base) + 3);
    skald_generate_random_addr(pdu);
    memcpy(pdu + 6, ad_base, sizeof(ad_base));

    void *data = (void *)(pdu + 6 + sizeof(ad_base));
    memset(data, 0x03, 3);

    skald_adv_start(&_adv);


    while (1) {
        ztimer_sleep(ZTIMER_MSEC, 1000);

        int raw = adc_sample(NRF52_VDDHDIV5, ADC_RES_12BIT);

        int bat = ((raw * BAT_MUL) / BAT_DIV) + 60;

        int f = bat / 1000;
        int g = (bat - (f * 1000)) / 100;
        int h = (bat - (f * 1000) - (g * 100)) / 10;

        // printf("raw ADC: %imV %i.%i%iV\n", bat, f, g, h);

        buf[0] = (uint8_t)f;
        buf[1] = (uint8_t)g;
        buf[2] = (uint8_t)h;

        // max 4095
        // min 0
        // range 0 -> 0.6V
        // 5V -> 0.5V  --->  0.6 / 4095

        memcpy(data, buf, 3);
    }

    return 0;
}
