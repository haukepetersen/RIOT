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

#define TYPE_GAS_RAW        (0x01)
#define TYPE_EL_RAW         (0x02)
#define TYPE_GAS            (0x03)
#define TYPE_EL             (0x04)

#define AD_STATIC           { 0x02, 0x01, 0x06, 0x0f, 0xff, 0xfe, 0xaf, 0x04 }

#define SAMPLE_RATE         (25u)
#define SAMPLING_DELAY      (1000000u / SAMPLE_RATE)

#define POWER_CONST         (37500000u)     /* MS_PER_HOUR / U per kWh */

#define CMA_HIGH            (25U)
#define CMA_LOW             (25U)
#define CMA_BAT             (10U)

#define INIT_HI_LO_OFFSET   (5U)           /* learned from experience... */

typedef struct __attribute__((packed)) {
    uint16_t sid;   /* hex 6-9 */
    uint16_t bat;   /* hex 10-13 */
    uint16_t cnt;   /* hex 14-17 */
    uint16_t pwr;   /* hex 18-21 */
    uint8_t hi;     /* hex 22-23 */
    uint8_t lo;     /* hex 24-25 */
    uint8_t th;     /* hex 26-27 */
} eldata_t;

typedef struct {
    uint16_t bat;
    uint16_t cnt;
    uint16_t pwr;
    uint32_t hi;
    uint32_t lo;
    uint32_t th;
    uint8_t last;
    uint32_t t_pulse;
} elstate_t;

static const uint8_t _ad_static[] = AD_STATIC;
static skald_ctx_t _adv;
static eldata_t *_data = NULL;
static elstate_t _state = { 0 , 0, 0, 0, 0, 0, 1, 0 };

/* cumulative moving average */
static unsigned _cma(uint32_t csm, uint32_t val, unsigned n)
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

static void _th_update(void)
{
    _state.th = (uint32_t)(_state.lo + ((_state.hi - _state.lo) / 2));
}

static uint32_t _el_sample(void)
{
    int ir_bg = adc_sample(A0, ADC_RES_8BIT);
    gpio_set(D_ON);
    xtimer_usleep(150);
    int ir_fg = adc_sample(A0, ADC_RES_8BIT);
    gpio_clear(D_ON);

    uint32_t val = (uint32_t)(ir_fg - ir_bg) * 1000;

    DEBUG("read: bg:%3i fg:%3i val:%3i -- hi:%6i lo:%6i th:%6i\n",
          ir_bg, ir_fg, (unsigned)val,
          (unsigned)_state.hi, (unsigned)_state.lo, (unsigned)_state.th);

    return val;
}

static void _el_read(void)
{
    int state_last = _state.last;
    uint32_t now = xtimer_now_usec();
    uint32_t val = _el_sample();

    LOG_INFO("v:%3i hi:%3i lo:%3i th:%3i\n", (int)val, (int)_state.hi,
             (int)_state.lo, (int)_state.th);

    if (val > _state.th) {
        _state.last = 1;
        _state.hi = _cma(_state.hi, val, CMA_HIGH);
    }
    else {
        _state.last = 0;
        _state.lo = _cma(_state.lo, val, CMA_LOW);
    }
    /* check if we saw a rising edge -> count new pulse and update pwr */
    if ((state_last == 0) && (_state.last == 1)) {
        _state.cnt++;
        _state.pwr = POWER_CONST / ((now - _state.t_pulse) / 1000);
        _state.t_pulse = now;
        DEBUG("--> pulse: cnt:%i pwr:%i\n", (int)_state.cnt, (int)_state.pwr);
    }

    _th_update();
}

static void _ad_set(void)
{
    memcpy(&_data->bat, &_state.bat, sizeof(uint16_t));
    memcpy(&_data->cnt, &_state.cnt, sizeof(uint16_t));
    memcpy(&_data->pwr, &_state.pwr, sizeof(uint16_t));
    _data->hi = (uint8_t)(_state.hi / 1000);
    _data->lo = (uint8_t)(_state.lo / 1000);
    _data->th = (uint8_t)(_state.th / 1000);
}

int main(void)
{
    /* initialize pins */
    adc_init(A0);
    adc_init(BAT_LINE);
    gpio_init(D_ON, GPIO_OUT);
    gpio_clear(D_ON);

    /* set initial hi/lo value */
    _state.hi = _el_sample();
    _state.lo = _state.hi - (1000 * INIT_HI_LO_OFFSET);
    _th_update();

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
    unsigned update_cnt = 0;
    xtimer_ticks32_t last_wakeup = xtimer_now_usec();

    while (1) {
        _el_read();

        if (++update_cnt == SAMPLE_RATE) {
            update_cnt = 0;
            _state.bat = _cma(_state.bat, _bat_read(), CMA_BAT);
            _ad_set();
        }

        xtimer_periodic_wakeup(&last_wakeup, SAMPLING_DELAY);
    }

    return 0;
}
