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
#include "random.h"
#include "thread_flags.h"
#include "qmc5883l.h"

#include "air.h"
#include "hilo.h"
#include "xenbat.h"
#include "ztimer.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define SAMPLE_RATE         (10U)
#define STARTUP_SAMPLES     (3000U)

#define FLAG_DRDY           (0x0001)

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

static const qmc5883l_params_t _mag_params = {
    .i2c = I2C_DEV(0),
    .pin_drdy = GPIO_PIN(1, 1),
    .odr = QMC5883L_ODR_10HZ,
    .rng = QMC5883L_RNG_8G,
    .osr = QMC5883L_OSR_64,
};

static qmc5883l_t _mag;

static void _on_mag_rdy(void *arg)
{
    thread_flags_set((thread_t *)arg, FLAG_DRDY);
}

int main(void)
{
    thread_t *t_main = (thread_t *)thread_get(thread_getpid());

    /* initialize battery readings */
    xenbat_init();

    /* setup BLE */
    airdata_t *data = air_init(TYPE_COUNTER, sizeof(airdata_t));
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
    hilo_init(&hilo);

    unsigned update_cnt = 0;

    for (unsigned i = 0; i < STARTUP_SAMPLES; i++) {
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
        int val = (int)sample[0];    /* we use the x-axis only */
        hilo_sample(&hilo, val);

        if (++update_cnt == SAMPLE_RATE) {
            update_cnt = 0;

            uint16_t bat = xenbat_sample();

            memcpy(&data->bat, &bat, sizeof(uint16_t));
            memcpy(&data->cnt, &hilo.cnt, sizeof(uint16_t));
            memcpy(&data->cnt_per_h, &hilo.cnt_per_h, sizeof(uint16_t));
            data->hi = (uint8_t)((hilo.hi + INT16_MIN) >> 8);
            data->lo = (uint8_t)((hilo.lo + INT16_MIN) >> 8);
            data->val = (uint8_t)((val + INT16_MIN) >> 8);
            data->state = hilo.state;
        }
    }

    return 0;
}
