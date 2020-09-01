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

#include "board.h"
#include "random.h"
#include "xtimer.h"
#include "thread_flags.h"
#include "periph/adc.h"
#include "periph/i2c.h"
#include "net/skald.h"
#include "qmc5883l.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define BAT_LINE            ADC_LINE(3)
#define BAT_OVERSAMPLE      (3U)
#define BAT_MUL             (95898)     /* (V_max * (R15 + R16) * 1000) / 100 */
#define BAT_DIV             (21504)     /* (R16 * ADC_max) / 100  */

#define BAT_UPDATE_ITVL     (20U)

#define MAG_I2C             I2C_DEV(0)
#define MAG_PIN             GPIO_PIN(1, 1)

#define FLAG_DRDY           (0x0001)


#define TYPE_GAS            (0x03)
#define AD_STATIC           { 0x02, 0x01, 0x06, 0x0a, 0xff, 0xfe, 0xaf, TYPE_GAS }

typedef struct __attribute__((packed)) {
    uint16_t sid;
    uint16_t bat;
    uint16_t cnt;
} tagdata_t;

static struct {
    uint16_t bat;
    uint16_t cnt;
} _state;

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

static void _on_mag_rdy(void *arg)
{
    thread_flags_set((thread_t *)arg, FLAG_DRDY);
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

int main(void)
{
    thread_t *t_main = (thread_t *)thread_get(thread_getpid());

    /* initialize pins */
    adc_init(BAT_LINE);

    /* setup BLE */
    skald_init();
    uint8_t *pdu = _adv.pkt.pdu;
    _adv.pkt.len = (6 + sizeof(_ad_static) + sizeof(tagdata_t));
    skald_generate_random_addr(pdu);
    memcpy(pdu + 6, _ad_static, sizeof(_ad_static));
    _data = (tagdata_t *)(pdu + 6 + sizeof(_ad_static));
    memset(_data, 0, sizeof(tagdata_t));
    /* generate session ID */
    random_bytes((uint8_t *)&_data->sid, 2);

    skald_adv_start(&_adv);

    int res = qmc5883l_init(&_mag, &_mag_params);
    if (res != QMC5883L_OK) {
        DEBUG("err init qmc\n");
        return 1;
    }
    res = qmc5883l_init_int(&_mag, _on_mag_rdy, t_main);
    if (res != QMC5883L_OK) {
        DEBUG("err init qmc int\n");
        return 1;
    }

    int16_t last = 0;
    unsigned bat_update_itvl = 0;

    while (1) {
        thread_flags_wait_all(FLAG_DRDY);
        int16_t sample[3];
        qmc5883l_read(&_mag, sample);

        DEBUG("x: %i\n", (int)sample[0]);

        if (++bat_update_itvl == BAT_UPDATE_ITVL) {
            bat_update_itvl = 0;
            _state.bat = _bat_read();
            memcpy(&_data->bat, &_state.bat, sizeof(uint16_t));
        }

        int16_t cur = sample[0];       /* read X component */
        if (last < 0 && (cur >= 0)) {
            _state.cnt++;
            memcpy(&_data->cnt, &_state.cnt, sizeof(uint16_t));
        }
        last = cur;
    }

    return 0;
}
