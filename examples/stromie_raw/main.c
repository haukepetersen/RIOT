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

#define AD_STATIC           { 0x02, 0x01, 0x06, 0x09, 0xff, 0xfe, 0xaf, TYPE_EL_RAW }

#define SAMPLE_RATE         (10u)
#define SAMPLING_DELAY      (1000000u / SAMPLE_RATE)
#define PULSE_THRESHOLD     (50u)
#define PULSE_OVERSAMPLE    (2u)

#define POWER_TIMEOUT       (300000u)   /* assume 0W power after 5 minutes */
#define POWER_CONST         (48000000u) /* MS_PER_HOUR / U per Wh */

typedef struct __attribute__((packed)) {
    uint16_t seq;
    uint16_t bat;
    uint8_t raw;
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

    int sum = 0;
    uint16_t seq = 0;
    unsigned cnt = 0;

    while (1) {
        int a = adc_sample(A0, ADC_RES_8BIT);
        sum += a;

        if (++cnt == SAMPLE_RATE) {
            sum /= SAMPLE_RATE;

            uint16_t v_bat = _bat_read();
            ++seq;

            byteorder_htobebufs((uint8_t *)&_data->seq, seq);
            byteorder_htobebufs((uint8_t *)&_data->bat, v_bat);
            _data->raw = (int)sum;

            sum = 0;
            cnt = 0;
        }

        xtimer_usleep(SAMPLING_DELAY);
    }

    return 0;
}
