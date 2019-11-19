/*
 * Copyright (C) 2019 Hauke Petersen <devel@haukepetersen.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     gasman
 * @{
 *
 * @file
 * @brief       Gas meter reading sensor
 *
 * @author      Hauke Petersen <devel@haukepetersen.de>
 *
 * @}
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "board.h"
#include "xtimer.h"
#include "byteorder.h"
#include "thread_flags.h"
#include "periph/adc.h"
#include "net/skald.h"
#include "qmc5883l.h"

#define BAT_LINE        ADC_LINE(3)
#define BAT_MUL         (95898)     /* (V_max * (R15 + R16) * 1000) / 100 */
#define BAT_DIV         (21504)     /* (R16 * ADC_max) / 100  */

#define MAG_I2C         I2C_DEV(0)
#define MAG_PIN         GPIO_PIN(1, 1)
#define OVR_SAMPLE      (10u)

#define FLAG_DRDY       (0x0001)

#define TYPE_GAS        (0x01)
#define TYPE_EL         (0x02)

#define AD_STATIC       { 0x02, 0x01, 0x06, 0x0e, 0xff, 0xfe, 0xaf, TYPE_GAS }

typedef struct __attribute__((packed)) {
    uint16_t seq;
    uint16_t bat;
    int16_t raw[3];
} tagdata_t;

static const qmc5883l_params_t _mag_params = {
    .i2c = MAG_I2C,
    .pin_drdy = MAG_PIN,
    .odr = QMC5883L_ODR_10HZ,
    .rng = QMC5883L_RNG_8G,
    .osr = QMC5883L_OSR_64,
};

static const uint8_t _ad_static[] = AD_STATIC;

static qmc5883l_t _mag;
static skald_ctx_t _adv;
static tagdata_t *_data = NULL;

static void _on_mag_sample(void *arg)
{
    thread_flags_set((thread_t *)arg, FLAG_DRDY);
}

static uint16_t _bat_read(void)
{
    int bat_raw = adc_sample(BAT_LINE, ADC_RES_10BIT);
    uint32_t v = ((bat_raw * BAT_MUL) / BAT_DIV);
    return (uint16_t)v;
}

int main(void)
{
    thread_t *t_main = (thread_t *)thread_get(thread_getpid());

    /* setup BLE */
    uint8_t *pdu = _adv.pkt.pdu;
    _adv.pkt.len = (6 + sizeof(_ad_static) + sizeof(tagdata_t));
    skald_generate_random_addr(pdu);
    memcpy(pdu + 6, _ad_static, sizeof(_ad_static));
    _data = (tagdata_t *)(pdu + 6 + sizeof(_ad_static));
    memset(_data, 0, sizeof(tagdata_t));

    /* initialize battery measurement */
    adc_init(BAT_LINE);

    puts("call init qmc");
    int res = qmc5883l_init(&_mag, &_mag_params);
    if (res != QMC5883L_OK) {
        puts("err init qmc");
        return 1;
    }
    puts("call init qmc int");
    res = qmc5883l_init_int(&_mag, _on_mag_sample, t_main);
    if (res != QMC5883L_OK) {
        puts("err init qmc int");
        return 1;
    }


    int32_t buf[3];
    memset(buf, 0, sizeof(buf));

    unsigned cnt = 0;
    uint16_t seq = 0;

    /* start advertising */
    skald_adv_start(&_adv);

    while (1) {
        LED0_TOGGLE;
        thread_flags_wait_all(FLAG_DRDY);
        int16_t sample[3];
        qmc5883l_read(&_mag, sample);
        ++cnt;

        for (int i = 0; i < 3; i++) {
            buf[i] += sample[i];
        }

        if (cnt == OVR_SAMPLE) {
            /* get battery voltage */
            uint16_t v_bat = _bat_read();
            int16_t raw[3];

            /* write advertising data */
            seq++;
            byteorder_htobebufs((uint8_t *)&_data->seq, seq);
            byteorder_htobebufs((uint8_t *)&_data->bat, v_bat);
            for (int i = 0; i < 3; i++) {
                raw[i] = (int16_t)(buf[i] / (int32_t)OVR_SAMPLE);
                byteorder_htobebufs((uint8_t *)&_data->raw[i], raw[i]);
            }

            /* reset buffer and counter */
            cnt = 0;
            memset(buf, 0, sizeof(buf));

            printf("data: %" PRIi16 " %" PRIi16 " %" PRIi16 " bat:%imV\n",
                   raw[0], raw[1], raw[2], (int)v_bat);
        }
    }

    return 0;
}
