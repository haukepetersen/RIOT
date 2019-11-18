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
#include "thread_flags.h"
#include "periph/i2c.h"
#include "net/skald.h"
#include "qmc5883l.h"

#define MAG_I2C         I2C_DEV(0)
#define MAG_PIN         GPIO_PIN(1, 1)
#define OVR_SAMPLE      (20u)

#define FLAG_DRDY       (0x0001)

#define TYPE_GAS        (0x01)
#define TYPE_EL         (0x02)

#define AD_STATIC       { 0x02, 0x01, 0x06, 0x0b, 0xff, 0xfe, 0xaf, TYPE_GAS }

typedef struct __attribute__((packed)) {
    int16_t raw[3];
    uint16_t seq;
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

int main(void)
{
    thread_t *t_main = (thread_t *)thread_get(thread_getpid());

    /* initialize supporting modules */
    // xtimer_init();
    // i2c_init(I2C_DEV(0));

    printf("foobar\n");

    /* setup BLE */
    skald_init();
    uint8_t *pdu = _adv.pkt.pdu;
    _adv.pkt.len = (6 + sizeof(_ad_static) + sizeof(tagdata_t));
    skald_generate_random_addr(pdu);
    memcpy(pdu + 6, _ad_static, sizeof(_ad_static));
    _data = (tagdata_t *)(pdu + 6 + sizeof(_ad_static));
    memset(_data, 0, sizeof(tagdata_t));

    skald_adv_start(&_adv);

    int res = qmc5883l_init(&_mag, &_mag_params);
    if (res != QMC5883L_OK) {
        puts("err init qmc");
        return 1;
    }
    res = qmc5883l_init_int(&_mag, _on_mag_sample, t_main);
    if (res != QMC5883L_OK) {
        puts("err init qmc int");
        return 1;
    }

    int32_t buf[3];
    memset(buf, 0, sizeof(buf));

    unsigned cnt = 0;
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
            for (int i = 0; i < 3; i++) {
                _data->raw[i] = (int16_t)(buf[i] / (int16_t)OVR_SAMPLE);
            }
            _data->seq++;
            printf("data: %" PRIi16 " %" PRIi16 " %" PRIi16 "\n", _data->raw[0], _data->raw[1], _data->raw[2]);
            memset(buf, 0, sizeof(buf));
            cnt = 0;
        }
    }

    return 0;
}
