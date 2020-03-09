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
 * @brief       Homelog node for reading light and temperature
 *
 * What to show in Grafana:
 * - light intensity in LUX
 * - (multiple) temperature readings
 * - battery level
 *
 * @author      Hauke Petersen <devel@haukepetersen.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "board.h"
#include "xtimer.h"
#include "byteorder.h"
#include "net/skald.h"
#include "periph/adc.h"


#include "ds18b20.h"
#include "onewire.h"
#include "bh1750fvi.h"

#define BAT_LINE            ADC_LINE(3)
#define BAT_MUL             (95898)     /* (V_max * (R15 + R16) * 1000) / 100 */
#define BAT_DIV             (21504)     /* (R16 * ADC_max) / 100  */

#define SAMPLE_ITVL         (1U)

#define TYPE_GAS_RAW        (0x01)
#define TYPE_EL_RAW         (0x02)
#define TYPE_GAS            (0x03)
#define TYPE_EL             (0x04)
#define TYPE_BAT            (0x05)
#define TYPE_LIGHT          (0x06)
#define TYPE_TEMP           (0x07)

#define AD_STATIC           { 0x02, 0x01, 0x06, 0x00, 0xff, 0xfe, 0xaf }
#define AD_POS_LEN          (BLE_ADDR_LEN + 3U)
#define AD_LEN_BASE         (3U)

typedef struct __attribute__((packed)) {
    uint16_t seq;
    uint8_t type_bat;
    uint16_t bat;
    uint8_t type_l;
    uint16_t light0;
    struct {
        uint8_t type_t;
        int16_t val;
    } temp[3];
} payload_t;

static const uint8_t _ad_static[] = AD_STATIC;
static skald_ctx_t _adv;

static bh1750fvi_t _light0;
static onewire_t _owi;
static ds18b20_t _temp[3];
static uint16_t _seq = 0;
static payload_t *_data = NULL;

static bh1750fvi_params_t _light_p = {
    .i2c = I2C_DEV(0),
    .addr = BH1750FVI_ADDR_PIN_LOW,
};

static onewire_params_t _onewire_p = {
    .pin = PARTICLE_MESH_D13,
    .pin_mode = GPIO_OUT,
};

static const char *_temp_roms[] = {
    "abc",
    "ccd",
    "uuu",
};


static uint16_t _bat_read(void)
{
    int bat_raw = adc_sample(BAT_LINE, ADC_RES_10BIT);
    uint32_t v = ((bat_raw * BAT_MUL) / BAT_DIV);
    return (uint16_t)v;
}

static void _sense(void)
{
        uint16_t v_bat = _bat_read();

        uint16_t light = bh1750fvi_sample(&_light0);

        int16_t temp[3];
        ds18b20_trigger_all(&_temp[0]);
        xtimer_usleep(DS18B20_T_CONV_12BIT);
        for (unsigned i = 0; i < 3; i++) {
            int res = ds18b20_read(&_temp[i], &temp[i]);
            if (res != DS18B20_OK) {
                temp[i] = -50;
            }
        }

        byteorder_htobebufs((uint8_t *)&_data->bat, v_bat);
        byteorder_htobebufs((uint8_t *)&_data->light0, light);
        for (unsigned i = 0; i < 3; i++) {
            byteorder_htobebufs((uint8_t *)&_data->temp[i].val, (uint16_t)temp[i]);
        }
        byteorder_htobebufs((uint8_t *)&_data->seq, ++_seq);
}

int main(void)
{
    int res;
    (void)res;

    /* setup BLE */
    uint8_t *pdu = _adv.pkt.pdu;
    _adv.pkt.len = (BLE_ADDR_LEN + sizeof(_ad_static) + sizeof(payload_t));
    skald_generate_random_addr(pdu);
    memcpy(pdu + BLE_ADDR_LEN, _ad_static, sizeof(_ad_static));
    pdu[AD_POS_LEN] = AD_LEN_BASE + sizeof(payload_t);

    /* initialize sensors */
    adc_init(BAT_LINE);
    res = bh1750fvi_init(&_light0, &_light_p);
    assert(res == 0);

    res = onewire_init(&_owi, &_onewire_p);
    assert(res == ONEWIRE_OK);

    for (unsigned i = 0; i < 3; i++) {
        ds18b20_params_t p;
        p.res = DS18B20_RES_12BIT;
        res = onewire_rom_from_str(&p.rom, _temp_roms[i]);
        assert(res == ONEWIRE_OK);
        res = ds18b20_init(&_temp[i], &_owi, &p);
        assert(res == DS18B20_OK);
    }

    /* initialize advertising data */
    _data = (payload_t *)(pdu + BLE_ADDR_LEN + sizeof(_ad_static));
    memset(_data, 0, sizeof(payload_t));
    _data->type_bat = TYPE_BAT;
    _data->type_l = TYPE_LIGHT;
    for (unsigned i = 0; i < 3; i++) {
        _data->temp[i].type_t = TYPE_TEMP;
    }

    /* get initial sensor reading */
    _sense();

    /* start advertising */
    skald_adv_start(&_adv);

    while (1) {
        _sense();
        xtimer_sleep(SAMPLE_ITVL);
    }

    return 0;
}
