/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       minimal RIOT application, intended as size test
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "xtimer.h"
#include "net/skald.h"
#include "periph/gpio.h"
#include "periph/adc.h"

#define A0              ADC_LINE(1)
#define D0              GPIO_PIN(0, 4)
#define A_RES           ADC_RES_8BIT

#define TYPE_GAS        (0x01)
#define TYPE_EL         (0x02)

#define AD_STATIC       { 0x02, 0x01, 0x06, 0x08, 0xff, 0xfe, 0xaf, TYPE_EL }

typedef struct __attribute__((packed)) {
    uint8_t d;
    uint8_t a;
    uint16_t seq;
} eldata_t;

static const uint8_t _ad_static[] = AD_STATIC;
static skald_ctx_t _adv;
static eldata_t *_data = NULL;

int main(void)
{
    /* initialize supporting modules */
    printf("foobar\n");

    /* setup BLE */
    // skald_init();
    uint8_t *pdu = _adv.pkt.pdu;
    _adv.pkt.len = (6 + sizeof(_ad_static) + sizeof(eldata_t));
    skald_generate_random_addr(pdu);
    memcpy(pdu + 6, _ad_static, sizeof(_ad_static));
    _data = (eldata_t *)(pdu + 6 + sizeof(_ad_static));
    memset(_data, 0, sizeof(eldata_t));


    adc_init(A0);
    gpio_init(D0, GPIO_IN);

    for (int i = 0; i < (int)_adv.pkt.len; i++) {
        printf("0x%02x ", (int)_adv.pkt.pdu[i]);
    }
    puts("");

    skald_adv_start(&_adv);

    uint16_t seq = 0;
    while (1) {
        int d = gpio_read(D0);
        int a = adc_sample(A0, ADC_RES_8BIT);
        printf("D:%i A:%4i\n", d, a);

        ++seq;
        _data->d = (uint8_t)d;
        _data->a = (uint8_t)a;
        memcpy(&_data->seq, &seq, 2);

        xtimer_usleep(1000 * 1000);
    }

    return 0;
}
