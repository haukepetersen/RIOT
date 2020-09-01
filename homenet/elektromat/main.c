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
#define STARTUP_SAMPLES     (5000U)

// #ifndef POWER_CONST
// #define POWER_CONST         (37500000u)     /* MS_PER_HOUR / U per kWh */
// #endif

#define TYPE_EL             (0x04)

typedef struct __attribute__((packed)) {
    uint16_t sid;   /* hex 6-9 */
    uint16_t bat;   /* hex 10-13 */
    uint16_t cnt;   /* hex 14-17 */
    uint16_t pwr;   /* hex 18-21 */
    uint8_t hi;     /* hex 22-23 */
    uint8_t lo;     /* hex 24-25 */
    uint8_t val;    /* hex 26-27 */
    uint8_t state;  /* hex 28-29 */
} airdata_t;

static uint16_t _cnt = 0;
static uint16_t _pwr = 0;
static uint32_t _t_last;

static int _ir_sample(void)
{
    int ir_bg = adc_sample(A0, ADC_RES_8BIT);
    gpio_set(D_ON);
    xtimer_usleep(150);
    int ir_fg = adc_sample(A0, ADC_RES_8BIT);
    gpio_clear(D_ON);
    return (ir_fg - ir_bg);
}

static void _on_pulse(void)
{
    uint32_t now = xtimer_now_usec();

    _cnt++;
    _pwr = POWER_CONST / ((now - _t_last) / 1000);
    _t_last = now;
}

int main(void)
{
    /* initialize pins */
    xenbat_init();
    adc_init(A0);
    gpio_init(D_ON, GPIO_OUT);
    gpio_clear(D_ON);

    /* setup BLE */
    airdata_t *data = air_init(TYPE_EL, sizeof(airdata_t));
    random_bytes((uint8_t *)&data->sid, 2);
    air_start();

    /* run sampling loop */
    hilo_t hilo;
    hilo_init(&hilo, _on_pulse);

    _t_last = xtimer_now_usec();
    unsigned update_cnt = 0;
    xtimer_ticks32_t last_wakeup = xtimer_now();

    /* learning phase: detect maxhi and minlo values */
    for (unsigned i = 0; i < STARTUP_SAMPLES; i++) {
        int val = _ir_sample();
        hilo_update(&hilo, val);
        xtimer_periodic_wakeup(&last_wakeup, SAMPLING_DELAY);
    }

    LOG_INFO("STARTUP DONE\n");

    while (1) {
        int val = _ir_sample();
        hilo_sample(&hilo, val);

        /* update advertising data */
        if (++update_cnt == SAMPLE_RATE) {
            update_cnt = 0;
            uint16_t bat = xenbat_sample();

            memcpy(&data->bat, &bat, sizeof(uint16_t));
            memcpy(&data->cnt, &_cnt, sizeof(uint16_t));
            memcpy(&data->pwr, &_pwr, sizeof(uint16_t));
            data->val = (uint8_t)val;
            data->hi = (uint8_t)hilo.hi;
            data->lo = (uint8_t)hilo.lo;
            data->state = hilo.state;
        }

        xtimer_periodic_wakeup(&last_wakeup, SAMPLING_DELAY);
    }

    return 0;
}
