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
 * @brief       Homelog node for reading an analog electrical power meter
 *
 * Strategy:
 * - sample the sensor with 10Hz
 * - on rising flank (2 consecutive readings above threshold):
 *   - take timestamp
 *   - increment counter
 *   - recalc
 *
 * -> max rotation: 1 per 2 seconds
 *
 *
 * What to show in Grafana:
 * - current power consumption (W)
 * - daily power consumption (kWh)
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

#define POWER_CONST         (37500000u)     /* MS_PER_HOUR / U per kWh */

#define STARTUP_SAMPLES     (5000U)
#define CMA_EL              (20000U)
#define CMA_BAT             (10U)

#define AD_STATIC           { 0x02, 0x01, 0x06, 0x10, 0xff, 0xfe, 0xaf, 0x04 }

typedef struct __attribute__((packed)) {
    uint16_t sid;   /* hex 6-9 */
    uint16_t bat;   /* hex 10-13 */
    uint16_t cnt;   /* hex 14-17 */
    uint16_t pwr;   /* hex 18-21 */
    uint8_t hi;     /* hex 22-23 */
    uint8_t lo;     /* hex 24-25 */
    uint8_t val;    /* hex 26-27 */
    uint8_t state;  /* hex 28-29 */
} eldata_t;

static struct {
    uint8_t hi;
    uint8_t lo;
    uint8_t maxhi;
    uint8_t minlo;
    uint8_t state;
} _th = { 0, 0, 0, UINT8_MAX, 0 };

static const uint8_t _ad_static[] = AD_STATIC;
static skald_ctx_t _adv;
static eldata_t *_data = NULL;

static uint16_t _bat_sample(void)
{
    unsigned raw = (unsigned)adc_sample(BAT_LINE, ADC_RES_10BIT);
    return (uint16_t)((raw * BAT_MUL) / BAT_DIV);
}

static uint8_t _el_sample(void)
{
    int ir_bg = adc_sample(A0, ADC_RES_8BIT);
    gpio_set(D_ON);
    xtimer_usleep(150);
    int ir_fg = adc_sample(A0, ADC_RES_8BIT);
    gpio_clear(D_ON);
    return (uint8_t)(ir_fg - ir_bg);
}

static void _th_update(int val)
{
    /* update maximal values */
    _th.minlo = (val < _th.minlo) ? val : _th.minlo;
    _th.maxhi = (val > _th.maxhi) ? val : _th.maxhi;

    /* calculate thresholds */
    uint8_t off = ((_th.maxhi - _th.minlo) / 3);
    _th.lo = _th.minlo + off;
    _th.hi = _th.maxhi - off;
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
    skald_adv_start(&_adv);

    /* run sampling loop */
    uint16_t cnt = 0;
    uint16_t pwr = 0;
    uint32_t t_pulse = xtimer_now_usec();

    unsigned update_cnt = 0;
    xtimer_ticks32_t last_wakeup = xtimer_now();

    for (unsigned i = 0; i < STARTUP_SAMPLES; i++) {
        int val = _el_sample();
        _th_update(val);
        LOG_INFO("v:%3i hi:%3i lo:%6i state:%i\n", val, _th.hi, _th.lo, _th.state);
        xtimer_periodic_wakeup(&last_wakeup, SAMPLING_DELAY);
    }

    /* calculate initial values from startup samples */
    LOG_INFO("STARTUP DONE: hi:%i lo:%i\n", _th.hi, _th.lo);

    while (1) {
        uint32_t now = xtimer_now_usec();
        int val = _el_sample();

        /* check for state change */
        if ((val < _th.lo) && (_th.state == 0)) {
            _th.state = 1;
            _th.minlo = val;

            cnt++;
            pwr = POWER_CONST / ((now - t_pulse) / 1000);
            t_pulse = now;
        }
        else if ((val > _th.hi) && (_th.state == 1)) {
            _th.state = 0;
            _th.maxhi = val;
        }

        /* update minlo and maxhi values */
        _th_update(val);
        LOG_INFO("v:%3i hi:%3i lo:%6i state:%i\n", val, _th.hi, _th.lo, _th.state);

        /* update advertising data */
        if (++update_cnt == SAMPLE_RATE) {
            update_cnt = 0;
            uint16_t bat = _bat_sample();

            memcpy(&_data->bat, &bat, sizeof(uint16_t));
            memcpy(&_data->cnt, &cnt, sizeof(uint16_t));
            memcpy(&_data->pwr, &pwr, sizeof(uint16_t));
            _data->val = val;
            _data->hi = _th.hi;
            _data->lo = _th.lo;
            _data->state = _th.state;
        }

        xtimer_periodic_wakeup(&last_wakeup, SAMPLING_DELAY);
    }

    return 0;
}
