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

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "board.h"
// #include "random.h"
#include "xtimer.h"
#include "thread_flags.h"
#include "periph/adc.h"

#include "bh1750fvi.h"
#include "bh1750fvi_params.h"

#include "net/skald.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

#define MOIST_NUMOF         (4U)
#define TEMP_NUMOF          (4U)
#define PUMP_NUMOF          (1U)

#define MOIST_0             ADC_LINE(0)

// #define SAMLING_DELAY       (1 * US_PER_SEC)
#define SAMLING_DELAY       (250000)

#define TYPE_BALCONEER      (0x10)
#define AD_LEN              (sizeof(airdata_t) + 4)

typedef struct __attribute__((packed)) {
    // uint16_t sid;
    uint16_t cnt;
    uint8_t moist[MOIST_NUMOF];
    // temp[TEMP_NUMOF];
    uint16_t illu;
    uint8_t pump;
} airdata_t;

// static struct {
//     uint16_t cnt;
// } _state;

static const uint8_t _ad_static[] = { 0x02, 0x01, 0x06,
                                      AD_LEN, 0xff, 0xfe, 0xaf, TYPE_BALCONEER };

static bh1750fvi_t _illu;
static skald_ctx_t _adv;


int main(void)
{
    airdata_t *airdata;
    // thread_t *t_main = (thread_t *)thread_get(thread_getpid());

    /* setup light sensor */
    int res = bh1750fvi_init(&_illu, &bh1750fvi_params[0]);
    if (res != BH1750FVI_OK) {
        DEBUG("[illu] Error: unable to initialize\n");
        return 1;
    }

    /* setup ADC */
    adc_init(MOIST_0);

    /* setup BLE */
    skald_init();
    uint8_t *pdu = _adv.pkt.pdu;
    _adv.pkt.len = (BLE_ADDR_LEN + sizeof(_ad_static) + sizeof(airdata_t));
    skald_generate_random_addr(pdu);
    memcpy(pdu + BLE_ADDR_LEN, _ad_static, sizeof(_ad_static));
    airdata = (airdata_t *)(pdu + BLE_ADDR_LEN + sizeof(_ad_static));
    memset(airdata, 0, sizeof(airdata_t));
    /* generate session ID */
    // random_bytes((uint8_t *)&_data->sid, 2);
    skald_adv_start(&_adv);

    DEBUG("Hello BLE\n");

    /* run the sampling loop */
    xtimer_ticks32_t last_wakeup = xtimer_now();

    while (1) {
        /* sample the light sensor (takes ~120ms) */
        uint16_t v_illu = bh1750fvi_sample(&_illu);
        memcpy(&airdata->illu, &v_illu, sizeof(uint16_t));

        int m = adc_sample(MOIST_0, ADC_RES_10BIT);
        DEBUG("[illu] v:%i [MOIST_0] v:%i\n", (int)v_illu, m);

        xtimer_periodic_wakeup(&last_wakeup, SAMLING_DELAY);
    }

    return 0;
}
