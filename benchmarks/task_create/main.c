/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     benchmarks
 * @{
 *
 * @file
 * @brief       Measuring task switching times
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "sched.h"
#include "thread.h"
#include "xtimer.h"

#define C_REPEAT        (10)

#define T_NUMOF         (SCHED_PRIO_LEVELS - 2)

static int iter[] = { 1, 100, 1000, 10000, 1500 };
static unsigned res[C_REPEAT];

static char stacks[SCHED_PRIO_LEVELS - 2][THREAD_STACKSIZE_DEFAULT];

static void *thread_generic(void *arg)
{
    int prio = (int)arg;

    kernel_pid_t pid = thread_getpid();
    printf("Thread %2i started - prio is %2i\n", (int)pid, prio);

    return NULL;
}

int main(void)
{
    uint32_t now;
    uint32_t done;

    for (int it = 0; it < (sizeof(iter) / sizeof(iter[0])); it++) {
        for (int r = 0; r < C_REPEAT; r++) {
            now = xtimer_now_usec();
            for (int i = 0; i < iter[it]; i++) {
                for (int t = 0; t < T_NUMOF; t++) {
                    thread_create(stacks[t], sizeof(stacks[t]), (char)t,
                                  THREAD_CREATE_WOUT_YIELD,
                                  thread_generic, (void *)t, "T");
                }
            }
            done = xtimer_now_usec();
            res[r] = (int)(done - now);
        }

        unsigned avg = 0;
        for (int i = 0; i < C_REPEAT; i++) {
            avg += res[i];
        }
        avg *= (1000 / C_REPEAT);
        unsigned per = avg / iter[it] / T_NUMOF;
        printf("Creation of %5i times %i threads: %u ns, %u ns per Thread\n",
                iter[it], (int)T_NUMOF, avg, per);
    }

    printf("this is main -> prio is %2i\n", THREAD_PRIORITY_MAIN);

    return 0;
}
