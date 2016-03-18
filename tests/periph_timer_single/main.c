/*
 * Copyright (C) 2015 Freie Universität Berlin
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
 * @brief       Peripheral timer test application
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "periph/timer.h"


#define TIM_SPEED           (1000000ul) /* try to run with 1MHz */

#define STEP                (10000U)
#define SUM                 (500)

static volatile int busy = 1;
static volatile int count = 0;

static void cb(void *arg, int chan)
{
    ++count;
    if (count == SUM) {
        count = 0;
        busy = 0;
    }

    timer_set(TIMER_DEV(0), 0, STEP);
}

int main(void)
{
    timer_init(TIMER_DEV(0), TIM_SPEED, cb, NULL);
    timer_set(TIMER_DEV(0), 0, STEP);

    while (1) {
        while (busy) {}
        busy = 1;
        puts("PING");
    }

    return 0;
}
