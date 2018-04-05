/*
 * Copyright (C) 2018 Freie Universität Berlin
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
 * @brief       Test the RIOT port of Gorm's timer queue abstraction
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <stdio.h>

#include "timex.h"
#include "random.h"
#include "net/gorm/ll.h"
#include "net/gorm/arch/evtim.h"

#define INTERVAL_SUPER              (1000UL * US_PER_MS)
#define INTERVAL_EVENT              (10UL * US_PER_MS)

event_queue_t gorm_ll_runqueue;

static uint32_t set_sum = 0;
static uint32_t act_sum = 0;
static uint32_t last;
static unsigned long cnt = 0;

void on_super(void *arg)
{
    gorm_arch_evtim_t *super = (gorm_arch_evtim_t *)arg;

    int diff;
    if (set_sum > act_sum) {
        diff = (int)(set_sum - act_sum);
        diff *= -1;
    }
    else {
        diff = (int)(act_sum - set_sum);
    }

    printf("check: diff is %i for %lu cycles\n", diff, cnt);
    gorm_arch_evtim_set_from_last(super, INTERVAL_SUPER);
}

void on_event(void *arg)
{
    gorm_arch_evtim_t *event = (gorm_arch_evtim_t *)arg;

    /* insert a random delay */
    uint32_t delay = random_uint32_range(0, 1000);
    for (; delay > 0; delay--) {
        __asm__ volatile ("nop");
    }

    uint32_t now = xtimer_now_usec();
    uint32_t time_past = (now - last);
    last = now;

    set_sum += INTERVAL_EVENT;
    act_sum += time_past;
    cnt++;

    gorm_arch_evtim_set_from_last(event, INTERVAL_EVENT);
}


int main(void)
{
    puts("Gorm EVT interface test");

    event_queue_init(&gorm_ll_runqueue);

    /* initialize supervision timer */
    gorm_arch_evtim_t super;
    gorm_arch_evtim_init(&super, on_super, &super);

    /* initialize event timer */
    gorm_arch_evtim_t event;
    gorm_arch_evtim_init(&event, on_event, &event);

    /* set supervision and event timers */
    gorm_arch_evtim_set_from_now(&event, INTERVAL_EVENT);
    last = (event.last - INTERVAL_EVENT);
    gorm_arch_evtim_set_from_now(&super, INTERVAL_SUPER);

    /* run event loop */
    event_loop(&gorm_ll_runqueue);

    /* never reached */
    return 0;
}
