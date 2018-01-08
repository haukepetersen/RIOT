/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *               2017 Hauke Petersen <hauke.petersen@fu-berlin.de>
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
 * @brief       timer test application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "msg.h"
#include "thread.h"
#include "xtimer.h"

#ifndef NUMOF
#define NUMOF   (25)
#endif

#define MSG_TYPE    (0xaffe)

static xtimer_ticks32_t to_default[NUMOF + 1];
static xtimer_ticks32_t to_relative[NUMOF + 1];
static kernel_pid_t pid_main;

static void on_timeout(void *arg)
{
    uint32_t num = (uint32_t)arg;
    msg_t m;
    m.type = MSG_TYPE;
    m.content.value = num;
    msg_send(&m, pid_main);
}

int main(void)
{
    puts("xtimer_set_relative test application.\n");

    pid_main = thread_getpid();
    msg_t m;
    uint32_t num = 0;

    /* allocate and pre-initialize a timer */
    xtimer_t t;
    t.callback = on_timeout;

    /* run the test loop for xtimer set */
    to_default[num++] = xtimer_now();
    while (num <= NUMOF) {
        t.arg = (void *)num;
        xtimer_set(&t, (US_PER_MS * num));

        msg_receive(&m);
        if (num != m.content.value) {
            puts("error: unexpected timeout message\n");
            return 1;
        }
        to_default[num++] = xtimer_now();
        printf("default, run %3i: %9" PRIu32 "\n", (num - 1), to_default[num - 1].ticks32);
    }

    puts("now running xtimer_set_relative");

    /* reset counter and reset the timer */
    num = 0;
    xtimer_reset(&t);

    /* run the test loop for xtimer set relative */
    to_relative[num++] = xtimer_now();
    while (num <= NUMOF) {
        t.arg = (void *)num;
        xtimer_set_relative(&t, (US_PER_MS * num));

        msg_receive(&m);
        if (num != m.content.value) {
            puts("error: unexpected timeout message\n");
            return 1;
        }
        to_relative[num++] = xtimer_now();
        printf("relative, run %3i: %9" PRIu32 "\n", (num - 1), to_default[num - 1].ticks32);
    }

    /* get results */
    printf("Start: %" PRIu32 "\n", to_default[0].ticks32);
    for (unsigned i = 1; i <= NUMOF; i++) {
        uint32_t diff = to_default[i].ticks32 - to_default[i - 1].ticks32;
        printf("  raw: %9" PRIu32 "\tdiff: %9" PRIu32 "\n",
               to_default[i].ticks32, diff);
    }


    return 0;
}

