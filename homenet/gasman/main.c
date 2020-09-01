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

#include "log.h"
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

#define SAMPLE_RATE         (10U)
#define FLAG_DRDY           (0x0001)

#define TYPE_GAS            (0x03)

typedef struct __attribute__((packed)) {
    uint16_t sid;
    uint16_t bat;
    uint16_t cnt;
    int8_t hi;     /* hex 18-19 */
    int8_t lo;     /* hex 20-21 */
    int8_t val;    /* hex 22-23 */
    uint8_t state;  /* hex 28-29 */
} airdata_t;

static const qmc5883l_params_t _mag_params = {
    .i2c = I2C_DEV(0),
    .pin_drdy = GPIO_PIN(1, 1),
    .odr = QMC5883L_ODR_10HZ,
    .rng = QMC5883L_RNG_8G,
    .osr = QMC5883L_OSR_64,
};

static const uint8_t _ad_static[] = AD_STATIC;

static qmc5883l_t _mag;
static uint16_t _cnt = 0;

static void _on_mag_rdy(void *arg)
{
    thread_flags_set((thread_t *)arg, FLAG_DRDY);
}

static void _on_pulse(void)
{
    _cnt++;
}


int main(void)
{
    thread_t *t_main = (thread_t *)thread_get(thread_getpid());

    /* setup BLE */
    airdata_t *data = air_init(TYPE_GAS, sizeof(airdata_t));
    random_bytes((uint8_t *)&data->sid, 2);
    air_start();

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

    /* start sampling */
    hilo_t hilo;
    hilo_init(&hilo, _on_pulse);

    unsigned update_cnt = 0;

    while (unsigned i = 0; i < STARTUP_SAMPLES; i++) {
        int16_t sample[3];
        thread_flags_wait_all(FLAG_DRDY);
        qmc5883l_read(&_mag, sample);
        hilo_update(&hilo, (int)sample[0]);
    }

    LOG_INFO("STARTUP_DONE\n");

    while (1) {
        int16_t sample[3];
        thread_flags_wait_all(FLAG_DRDY);
        qmc5883l_read(&_mag, sample);
        int val = sample[0];    /* we use the x-axis only */
        hilo_sample(&hilo, val);

        if (++update_cnt == SAMPLE_RATE) {
            update_cnt = 0;
            uint16_t bat = xenbat_sample();

            memcpy(&data->bat, &bat, sizeof(uint16_t));
            memcpy(&data->cnt, &cnt, sizeof(uint16_t));
            data->val = (int8_t)(val >> 8);
            data->hi = (int8_t)(hilo.hi >> 8);
            data->lo = (int8_t)(hilo.lo >> 8);
            data->state = hilo.state;

        }
    }

    return 0;
}
