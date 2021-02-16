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
#include "ztimer.h"
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

#define SAMPLE_RATE         (1 * 25u)
#define AVG_RATE            (60 * 25u)
#define SAMPLING_DELAY      (1000u / SAMPLE_RATE)
#define STARTUP_SAMPLES     (5000U)

/**
 * the POWER_CONST is used to calculate current power usage:
 * POWER_CONSTANT [ms * W] / duration last rotation [ms] := power usage [W]
 *
 * It is computed as:
 * milliseconds per hour * watt in kilowatt / turns per kwh
 */
// #define MS_PER_H            (60UL * 60UL * MS_PER_SEC)
// #define W_PER_KW            (1000LU)
// #define POWER_CONST         ((MS_PER_H * W_PER_KW) / PWR_U_KWH)

#define TYPE_COUNTER        (0x06)

typedef struct __attribute__((packed)) {
    uint16_t sid;           /* hex 6-9 */
    uint16_t bat;           /* hex 10-13 */
    uint16_t cnt;           /* hex 14-17 */
    uint16_t cnt_per_h;     /* hex 18-21 */
    uint8_t hi;             /* hex 22-23 */
    uint8_t lo;             /* hex 24-25 */
    uint8_t val;            /* hex 26-27 */
    uint8_t state;          /* hex 28-29 */
} airdata_t;

// static uint16_t _cnt = 0;
// static uint16_t _pwr = 0;
// static uint32_t _t_last;

static int _ir_sample(void)
{
    // int ir_bg = adc_sample(A0, ADC_RES_8BIT);
    gpio_set(D_ON);
    // xtimer_usleep(150);
    int ir_fg = adc_sample(A0, ADC_RES_8BIT);
    gpio_clear(D_ON);
    return ir_fg;
}

int main(void)
{
    /* initialize pins */
    xenbat_init();
    adc_init(A0);
    gpio_init(D_ON, GPIO_OUT);
    gpio_clear(D_ON);

    /* setup BLE */
    airdata_t *data = air_init(TYPE_COUNTER, sizeof(airdata_t));
    random_bytes((uint8_t *)&data->sid, 2);
    air_start();

    /* run sampling loop */
    hilo_t hilo;
    hilo_init(&hilo);

    unsigned update_cnt = 0;
    uint32_t last_wakeup = ztimer_now(ZTIMER_MSEC);

    /* learning phase: detect maxhi and minlo values */
    for (unsigned i = 0; i < STARTUP_SAMPLES; i++) {
        int val = _ir_sample();
        hilo_update(&hilo, val);
        ztimer_periodic_wakeup(ZTIMER_MSEC, &last_wakeup, SAMPLING_DELAY);
    }

    LOG_INFO("STARTUP DONE\n");

    while (1) {
        int val = _ir_sample();
        hilo_sample(&hilo, val);

        /* update advertising data */
        if ((update_cnt % SAMPLE_RATE) == 0) {
            memcpy(&data->cnt, &hilo.cnt, sizeof(uint16_t));
            data->val = (uint8_t)val;
            data->hi = (uint8_t)hilo.hi;
            data->lo = (uint8_t)hilo.lo;
            data->state = hilo.state;
        }

        if ((update_cnt % AVG_RATE) == 0) {
            hilo_calc_avg(&hilo);
            memcpy(&data->cnt_per_h, &hilo.cnt_per_h, sizeof(uint16_t));

            uint16_t bat = xenbat_sample();
            memcpy(&data->bat, &bat, sizeof(uint16_t));
        }

        ztimer_periodic_wakeup(ZTIMER_MSEC, &last_wakeup, SAMPLING_DELAY);
    }

    return 0;
}
