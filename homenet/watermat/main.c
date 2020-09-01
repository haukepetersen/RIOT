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
 * @brief       Homelog node for reading an analog water power meter
 *
 * Strategy:
 * - sample the sensor with 10Hz
 * - on rising flank (2 consecutive readings above threshold):
 *   - take timestamp
 *   - increment counter
 *   - recalc
 *
 * @author      Hauke Petersen <devel@haukepetersen.de>
 *
 * @}
 */

#include <stdint.h>
#include <string.h>

#include "log.h"
#include "xtimer.h"
#include "random.h"
#include "periph/gpio.h"
#include "periph/adc.h"

#include "air.h"
#include "hilo.h"
#include "xenbat.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define A0                  ADC_LINE(1)
#define D_ON                GPIO_PIN(0, 31)

#define SAMPLE_RATE         (25u)
#define SAMPLING_DELAY      (1000000u / SAMPLE_RATE)
#define STARTUP_SAMPLES     (7500U)

#define TYPE_WAT            (0x05)

typedef struct __attribute__((packed)) {
    uint16_t sid;   /* hex 6-9 */
    uint16_t bat;   /* hex 10-13 */
    uint16_t cnt;   /* hex 14-17 */
    uint8_t hi;     /* hex 18-19 */
    uint8_t lo;     /* hex 20-21 */
    uint8_t val;    /* hex 22-23 */
    uint8_t state;  /* hex 28-29 */
} airdata_t;

static uint16_t _cnt = 0;

static uint8_t _ir_sample(void)
{
    int ir_bg = adc_sample(A0, ADC_RES_8BIT);
    gpio_set(D_ON);
    xtimer_usleep(150);
    int ir_fg = adc_sample(A0, ADC_RES_8BIT);
    gpio_clear(D_ON);
    return (uint8_t)(ir_fg - ir_bg);
}

static void _on_pulse(void)
{
    _cnt++;
}

int main(void)
{
    /* initialize pins */
    xenbat_init();
    adc_init(A0);
    gpio_init(D_ON, GPIO_OUT);
    gpio_clear(D_ON);

    /* setup BLE */
    airdata_t *data = air_init(TYPE_WAT, sizeof(airdata_t));
    random_bytes((uint8_t *)&data->sid, 2);
    air_start();

    /* run sampling loop */
    hilo_t hilo;
    hilo_init(&hilo, _on_pulse);

    unsigned update_cnt = 0;
    xtimer_ticks32_t last_wakeup = xtimer_now();

    /* learning phase: detect maxhi and minlo values */
    for (unsigned i = 0; i < STARTUP_SAMPLES; i++) {
        uint8_t val = _ir_sample();
        hilo_update(&hilo, val);
        xtimer_periodic_wakeup(&last_wakeup, SAMPLING_DELAY);
    }

    LOG_INFO("STARTUP DONE\n");

    while (1) {
        uint8_t val = _ir_sample();
        hilo_sample(&hilo, val);

        /* update advertising data */
        if (++update_cnt == SAMPLE_RATE) {
            update_cnt = 0;
            uint16_t bat = xenbat_sample();

            memcpy(&data->bat, &bat, sizeof(uint16_t));
            memcpy(&data->cnt, &cnt, sizeof(uint16_t));
            data->val = (uint8_t)val;
            data->hi = (uint8_t)hilo.hi;
            data->lo = (uint8_t)hilo.lo;
            data->state = hilo.state;
        }

        xtimer_periodic_wakeup(&last_wakeup, SAMPLING_DELAY);
    }

    return 0;
}
