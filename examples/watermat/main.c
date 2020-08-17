/*
 * Copyright (C) 2019 Hauke Petersen <devel@haukepeterse.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     homelog
 * @{
 *
 * @file
 * @brief       Homelog node for reading an analog water power meter
 *
 * Strategy:
 * - sample the sensor with 10Hz
 * - on rising flank (2 consecutive readings above threshold):
 *   - take timestamp
 *   - increment counter
 *   - recalc
 *
 * @author      Hauke Petersen <devel@haukepetersen.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "log.h"
#include "xtimer.h"
#include "random.h"
#include "byteorder.h"
#include "net/skald.h"
#include "periph/gpio.h"
#include "periph/adc.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define BAT_LINE            ADC_LINE(3)
#define BAT_OVERSAMPLE      (1U)        /* TODO: needed when doing CMA? */
#define BAT_MUL             (95898)     /* (V_max * (R15 + R16) * 1000) / 100 */
#define BAT_DIV             (21504)     /* (R16 * ADC_max) / 100  */

#define A0                  ADC_LINE(1)
#define D_ON                GPIO_PIN(0, 31)


#define SAMPLE_RATE         (25u)
#define SAMPLING_DELAY      (1000000u / SAMPLE_RATE)

#define STARTUP_SAMPLES     (5000U)
#define CMA_EL              (20000U)
#define CMA_BAT             (10U)

#define AD_STATIC           { 0x02, 0x01, 0x06, 0x0d, 0xff, 0xfe, 0xaf, 0x05 }

typedef struct __attribute__((packed)) {
    uint16_t sid;   /* hex 6-9 */
    uint16_t bat;   /* hex 10-13 */
    uint16_t cnt;   /* hex 14-17 */
    uint8_t hi;     /* hex 18-19 */
    uint8_t lo;     /* hex 20-21 */
    uint8_t val;    /* hex 22-23 */
} eldata_t;

static const uint8_t _ad_static[] = AD_STATIC;
static skald_ctx_t _adv;
static eldata_t *_data = NULL;

/* cumulative moving average */
static uint64_t _cma(uint64_t csm, uint32_t val, unsigned n)
{
    return ((csm * n) + val) / (n + 1);
}

static uint16_t _bat_read(void)
{
    int bat_raw = 0;

    for (unsigned i = 0; i < BAT_OVERSAMPLE; i++) {
        bat_raw +=  adc_sample(BAT_LINE, ADC_RES_10BIT);
    }
    uint32_t v = ((bat_raw * BAT_MUL) / BAT_DIV) / BAT_OVERSAMPLE;
    return (uint16_t)v;
}

static uint32_t _el_sample(void)
{
    int ir_bg = adc_sample(A0, ADC_RES_8BIT);
    gpio_set(D_ON);
    xtimer_usleep(150);
    int ir_fg = adc_sample(A0, ADC_RES_8BIT);
    gpio_clear(D_ON);

    return (uint32_t)(ir_fg - ir_bg) * 1000;
}

int main(void)
{
    /* initialize pins */
    adc_init(A0);
    adc_init(BAT_LINE);
    gpio_init(D_ON, GPIO_OUT);
    gpio_clear(D_ON);

    /* setup BLE */
    uint8_t *pdu = _adv.pkt.pdu;
    _adv.pkt.len = (6 + sizeof(_ad_static) + sizeof(eldata_t));
    skald_generate_random_addr(pdu);
    memcpy(pdu + 6, _ad_static, sizeof(_ad_static));
    _data = (eldata_t *)(pdu + 6 + sizeof(_ad_static));
    memset(_data, 0, sizeof(eldata_t));
    /* create session id */
    random_bytes((uint8_t *)&_data->sid, 2);

    /* initialize power meter sampling input */
    skald_adv_start(&_adv);


    /* run sampling loop */
    uint16_t bat = 0;
    uint16_t cnt = 0;
    uint64_t hi = 0;

    uint64_t lo = 500000;
    int state = 0;

    unsigned update_cnt = 0;
    xtimer_ticks32_t last_wakeup = xtimer_now();

    for (unsigned i = 0; i < STARTUP_SAMPLES; i++) {
        uint32_t val = _el_sample();
        hi += val;
        if (val < lo) {
            lo = val;
        }
        LOG_INFO("v:%6u hi:%6u lo:%6u state:%i\n",
                 (unsigned)val, (unsigned)hi, (unsigned)lo, (unsigned)state);
        xtimer_periodic_wakeup(&last_wakeup, SAMPLING_DELAY);
    }

    /* calculate initial values from startup samples */
    hi = hi / STARTUP_SAMPLES;
    lo = lo + ((hi - lo) / 2);
    LOG_INFO("STARTUP DONE: hi:%u lo:%u\n", (unsigned)hi, (unsigned)lo);

    while (1) {
        uint32_t val = _el_sample();

        /* update hysteresis */
        hi = _cma(hi, val, CMA_EL);
        if (val < hi) {
            lo = _cma(lo, val, CMA_EL);
        }

        /* check for state change */
        if ((val < lo) && (state == 0)) {
            cnt++;
            state = 1;
        }
        else if ((val > hi) && (state == 1)) {
            state = 0;
        }

        LOG_INFO("v:%6u hi:%6u lo:%6u state:%i\n",
                 (unsigned)val, (unsigned)hi, (unsigned)lo, (unsigned)state);

        if (++update_cnt == SAMPLE_RATE) {
            update_cnt = 0;
            bat = (uint16_t)_cma(bat, _bat_read(), CMA_BAT);

            memcpy(&_data->bat, &bat, sizeof(uint16_t));
            memcpy(&_data->cnt, &cnt, sizeof(uint16_t));
            _data->hi = (uint8_t)(hi / 1000);
            _data->lo = (uint8_t)(lo / 1000);
            _data->val = (uint8_t)(val / 1000);
        }

        xtimer_periodic_wakeup(&last_wakeup, SAMPLING_DELAY);
    }

    return 0;
}
