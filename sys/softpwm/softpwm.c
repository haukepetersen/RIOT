/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     sys_softpwm
 * @{
 *
 * @file
 * @brief       Software PWM implementation
 *
 * @author      Hauke Petersen <mail@haukepetersen.de>
 *
 * @}
 */

#include "softpwm.h"


static void event(void *arg)
{
    softpwm_t *pwm = (softpwm_t *)arg;

    pwm->state ^= 1;
    if (pwm->state == 0 && pwm->update) {
        pwm->state ^= 2;
        pwm->update = 0;
    }

    if (pwm->dc[pwm->state]) {
        xtimer_set(&(pwm->timer), pwm->dc[pwm->state]);
        gpio_toggle(pwm->pin);
    }
    else {
        xtimer_set(&(pwm->timer), pwm->dc[(pwm->state ^ 1)]);
    }
}

uint32_t softpwm_init(softpwm_t *dev, gpio_t pin, uint32_t freq, uint16_t steps)
{
    /* check parameters */
    if (freq * steps > 1000000) {
        return 0;
    }

    /* initialize the output pin */
    gpio_init(pin, GPIO_DIR_OUT, GPIO_NOPULL);
    /* initialize the time-base */
    dev->res = (1000000 / (freq * steps));
    dev->dc[0] = steps * dev->res;
    dev->dc[1] = 0;
    memset(dev->dc, 0, 4 * sizeof(uint16_t));
    dev->state = 0;
    dev->update = 0;
    /* initialize the timer */
    dev->timer.callback = event;
    dev->timer.arg = (void *)dev;

    /* start the PWM */
    softpwm_start(dev);
}

void softpwm_set(softpwm_t *dev, uint16_t dc)
{
    uint16_t steps = (dev->dc[0] + dev->dc[1]);
    int i = (pwm->state & 2) ^ 2;

    if (dc > steps) {
        dc = steps;
    }
    pwm->dc[i] = dc * res;
    pwm->dc[i + 1] = (steps - dc) * res;
    pwm->update = 1;
}

void softpwm_start(softpwm_t *dev)
{
    xtimer_set(&(dev->timer), dev->dc[dev->state]);
}

void softpwm_stop(softpwm_t *dev)
{
    xtimer_remove(&(dev->timer));
}
