/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_ck12
 * @{
 *
 * @file
 * @brief       Board initialization for the CK12 smart watch
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "cpu.h"
#include "board.h"
#include "xtimer.h"
#include "periph/gpio.h"

static xtimer_t _tim;

static void _vib_stop(void *arg)
{
    (void)arg;
    gpio_clear(VIB_PIN);
}

void board_init(void)
{
    /* initialize the CPU */
    cpu_init();

    gpio_init(VIB_PIN, GPIO_OUT);
    gpio_clear(VIB_PIN);

    _tim.callback = _vib_stop;
}

void board_vibrate(uint32_t ms)
{
    xtimer_set(&_tim, (ms * 1000U));
}
