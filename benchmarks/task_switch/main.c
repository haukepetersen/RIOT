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

#define REPEAT          (10000000)

#define T_NUMOF         (SCHED_PRIO_LEVELS - 2)


static char stacks[SCHED_PRIO_LEVELS - 2][THREAD_STACKSIZE_DEFAULT];

static kernel_pid_t sleeper_pid;

// static void *thread_generic(void *arg)
// {
//     int prio = (int)arg;

//     kernel_pid_t pid = thread_getpid();
//     printf("Thread %2i started - prio is %2i\n", (int)pid, prio);

//     return NULL;
// }

static void *thread_sleeper(void *arg)
{
    // int foo = 0;
    int prio = (int)arg;
    kernel_pid_t pid = thread_getpid();
    printf("Thread %2i started - prio is %2i - SLEEPER\n", (int)pid, prio);

    while (1) {
        thread_sleep();
        // printf("SLEEPER %i\n", ++foo);
    }

    return NULL;
}

static void *thread_waker(void *arg)
{
    uint32_t now;
    uint32_t done;
    // int foo = 0;

    int prio = (int)arg;
    kernel_pid_t pid = thread_getpid();
    printf("Thread %2i started - prio is %2i - WAKER\n", (int)pid, prio);


    now = xtimer_now_usec();
    for (int i = 0; i < REPEAT; i++) {
        // printf("WAKER %i\n", ++foo);
        thread_wakeup(sleeper_pid);
    }
    done = xtimer_now_usec();

    puts("DONEDONEDONE");
    printf("DONE: sum %i for %i cycles\n", (int)(done - now), REPEAT);

    return NULL;
}

int main(void)
{

    // for (int t = 0; t < T_NUMOF; t++) {
    //     thread_create(stacks[t], sizeof(stacks[t]), (char)t,
    //                   THREAD_CREATE_WOUT_YIELD,
    //                   thread_generic, (void *)t, "T");
    // }

    sleeper_pid = thread_create(stacks[0], sizeof(stacks[0]), (char)0,
                  THREAD_CREATE_WOUT_YIELD,
                  thread_sleeper, (void *)0, "SLEEPER");
    thread_create(stacks[1], sizeof(stacks[1]), (char)1,
                  THREAD_CREATE_WOUT_YIELD,
                  thread_waker, (void *)1, "WAKER");

    printf("this is main -> prio is %2i\n", THREAD_PRIORITY_MAIN);

    thread_yield();

    while (1) {
        xtimer_now();
    }

    return 0;
}
