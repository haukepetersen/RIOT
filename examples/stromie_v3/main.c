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

#include "xtimer.h"
#include "byteorder.h"
#include "net/skald.h"
#include "periph/gpio.h"
#include "periph/adc.h"

#define BAT_LINE            ADC_LINE(3)
#define BAT_MUL             (95898)     /* (V_max * (R15 + R16) * 1000) / 100 */
#define BAT_DIV             (21504)     /* (R16 * ADC_max) / 100  */

#define A0                  ADC_LINE(1)
#define D_ON                GPIO_PIN(0, 31)

#define TYPE_GAS_RAW        (0x01)
#define TYPE_EL_RAW         (0x02)
#define TYPE_GAS            (0x03)
#define TYPE_EL             (0x04)

#define AD_STATIC           { 0x02, 0x01, 0x06, 0x0f, 0xff, 0xfe, 0xaf, 0x04 }

#define SAMPLE_RATE         (20u)
#define SAMPLING_DELAY      (1000000u / SAMPLE_RATE)

#define POWER_CONST         (48000000u)     /* MS_PER_HOUR / U per kWh */

#define CMA_HIGH            (25U)
#define CMA_LOW             (100U)

#define TH_INIT             (50U)
#define HI_INIT             (100U)
#define LO_INIT             (5U)

typedef struct __attribute__((packed)) {
    uint16_t seq;
    uint16_t bat;
    uint16_t pulse_cnt;
    uint16_t power;
    uint8_t low;
    uint8_t th;
    uint8_t high;
} eldata_t;

static const uint8_t _ad_static[] = AD_STATIC;
static skald_ctx_t _adv;
static eldata_t *_data = NULL;

#if 0
#define HIGH_CNT            (50U)

static uint8_t _tmax = 0;
static uint8_t _tmin = UINT8_MAX;
static uint8_t _thold = UINT8_MAX;
#endif

/* cumulative moving average */
static unsigned _cma(unsigned csm, uint8_t val, unsigned n)
{
    return ((csm * n) + val) / (n + 1);
}

static uint16_t _bat_read(void)
{
    int bat_raw = adc_sample(BAT_LINE, ADC_RES_10BIT);
    uint32_t v = ((bat_raw * BAT_MUL) / BAT_DIV);
    return (uint16_t)v;
}

int main(void)
{
    /* setup BLE */
    uint8_t *pdu = _adv.pkt.pdu;
    _adv.pkt.len = (6 + sizeof(_ad_static) + sizeof(eldata_t));
    skald_generate_random_addr(pdu);
    memcpy(pdu + 6, _ad_static, sizeof(_ad_static));
    _data = (eldata_t *)(pdu + 6 + sizeof(_ad_static));
    memset(_data, 0, sizeof(eldata_t));
    /* initialize power meter sampling input */
    skald_adv_start(&_adv);

    /* initialize pins */
    adc_init(A0);
    adc_init(BAT_LINE);
    gpio_init(D_ON, GPIO_OUT);
    gpio_clear(D_ON);

    /* determine the state */
    uint8_t th = TH_INIT;
    unsigned hi = HI_INIT;
    unsigned lo = LO_INIT;

    unsigned update_cnt = 0;
    unsigned state = 0;
    uint16_t seq = 0;
    uint16_t pulse_cnt = 0;
    uint32_t last_pulse = 0;

    while (1) {
        int ir_bg = adc_sample(A0, ADC_RES_8BIT);
        gpio_set(D_ON);
        xtimer_usleep(150);
        int ir_fg = adc_sample(A0, ADC_RES_8BIT);
        gpio_clear(D_ON);

        uint8_t val = (uint8_t)(ir_fg - ir_bg);
        printf("sample: bg:%i fg:%i\n", ir_bg, ir_fg);

        if (val > th) {
            ++state;
            if (state == 1) {
                ++state;    /* make sure we only increment once per pulse */

                ++pulse_cnt;
                uint32_t now = xtimer_now_usec();
                uint32_t dur_ms = (now - last_pulse) / 1000;
                last_pulse = now;
                uint16_t power = POWER_CONST / dur_ms;
            }
            hi = _cma(hi, val, CMA_HIGH);
        }
        else {
            lo = _cma(lo, val, CMA_LOW);
            state = 0;
        }
        th = (uint8_t)(lo + ((hi - lo) / 2));

        if (++update_cnt == SAMPLE_RATE) {
            ++seq;
            uint16_t v_bat = _bat_read();

            byteorder_htobebufs((uint8_t *)&_data->seq, seq);
            byteorder_htobebufs((uint8_t *)&_data->bat, v_bat);
            byteorder_htobebufs((uint8_t *)&_data->pulse_cnt, (uint16_t)ir_bg);
            byteorder_htobebufs((uint8_t *)&_data->power, (uint16_t)ir_fg);
            _data->low = lo;
            _data->th = th;
            _data->high = hi;
        }

        xtimer_usleep(SAMPLING_DELAY);
    }

    return 0;
}
