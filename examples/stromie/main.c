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
 *
 * What to show in Grafana:
 * - current power consumption (W)
 * - daily power consumption (kWh)
 *
 * @author      Hauke Petersen <devel@haukepetersen.de>
 *
 * @}
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "xtimer.h"
#include "byteorder.h"
#include "net/skald.h"
#include "periph/gpio.h"
#include "periph/adc.h"

#define BAT_LINE            ADC_LINE(3)
#define BAT_MUL             (95898)     /* (V_max * (R15 + R16) * 1000) / 100 */
#define BAT_DIV             (21504)     /* (R16 * ADC_max) / 100  */

#define A0                  ADC_LINE(1)
#define A_RES               ADC_RES_8BIT

#define TYPE_GAS_RAW        (0x01)
#define TYPE_EL_RAW         (0x02)
#define TYPE_GAS            (0x03)
#define TYPE_EL             (0x04)

#define AD_STATIC           { 0x02, 0x01, 0x06, 0x0c, 0xff, 0xfe, 0xaf, TYPE_EL }

#define SAMPLE_RATE         (10u)
#define SAMPLING_DELAY      (1000000u / SAMPLE_RATE)
#define PULSE_THRESHOLD     (18u)
#define PULSE_OVERSAMPLE    (2u)

#define POWER_TIMEOUT       (300000u)   /* assume 0W power after 5 minutes */
#define POWER_CONST         (48000000u) /* MS_PER_HOUR / U per Wh */

typedef struct __attribute__((packed)) {
    uint16_t seq;
    uint16_t bat;
    uint16_t pulse_cnt;
    uint16_t power;
} eldata_t;

static const uint8_t _ad_static[] = AD_STATIC;
static skald_ctx_t _adv;
static eldata_t *_data = NULL;


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

    /* initialize battery measurement */
    adc_init(BAT_LINE);

    /* initialize power meter sampling input */
    adc_init(A0);
    skald_adv_start(&_adv);

    unsigned state = 0;
    uint32_t pulse_last = xtimer_now_usec();
    uint16_t seq = 0;
    uint16_t pulse_cnt = 0;
    uint16_t power;

    while (1) {
        int a = adc_sample(A0, ADC_RES_8BIT);

        if ((unsigned)a > PULSE_THRESHOLD) {
            LED0_ON;
            ++state;
        }
        else {
            LED0_OFF;
            state = 0;
        }

        if (state == PULSE_OVERSAMPLE) {
            uint32_t now = xtimer_now_usec();
            uint32_t itvl = (now - pulse_last) / 1000;      /* in ms */
            pulse_last = now;

            memcpy(&_data->seq, &seq, 2);
            memcpy(&_data->pulse_cnt, &pulse_cnt, 2);
            memcpy(&_data->power, &power, 2);
        }
        else if ((itvl > POWER_TIMEOUT) && (power != 0)) {
            ++seq;
            uint16_t v_bat = _bat_read();
            ++pulse_cnt;
            power = (uint16_t)(POWER_CONST / itvl);

            byteorder_htobebufs((uint8_t *)&_data->seq, seq);
            byteorder_htobebufs((uint8_t *)&_data->bat, v_bat);
            byteorder_htobebufs((uint8_t *)&_data->pulse_cnt, pulse_cnt);
            byteorder_htobebufs((uint8_t *)&_data->power, power);
        }

        xtimer_usleep(SAMPLING_DELAY);
    }

    return 0;
}
