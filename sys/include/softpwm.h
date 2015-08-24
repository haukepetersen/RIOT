/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_softpwm Soft PWM
 * @ingroup     sys
 * @brief       PWM implementation in software using the xtimer
 *
 * @{
 * @file
 * @brief       Software PWM interface definition
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef SOFTPWM_H
#define SOFTPWM_H

#include <stdint.h>

#include "xtimer.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    gpio_t pin;
    xtimer_t timer;
    uint16_t dc[4];
    uint16_t res;
    uint8_t state;
    uint8_t update;
} softpwm_t;


int softpwm_init(softpwm_t *dev, gpio_t pin, uint32_t freq, uint16_t steps);

void softpwm_set(softpwm_t *dev, uint16_t value);

void softpwm_start(softpwm_t *dev);

void softpwm_stop(softpwm_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* SOFTPWM_H */
/** @} */
