/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
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
 * @brief       wtimer re-set test application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <stdio.h>

#include "thread.h"
#include "wtimer.h"
extern void wtimer_dump_all(void);
int main(void)
{
    printf("This test tests re-setting of an already active timer.\n");
    printf("It should print three times \"now=<value>\", with values"
           " approximatly 100ms (100000us) apart.\n");

    wtimer_t wtimer;
    wtimer_t wtimer2;

    kernel_pid_t me = thread_getpid();

    wtimer_set_wakeup(&wtimer, 1<<31, me);
    wtimer_set_wakeup(&wtimer2, 1<<31, me);

    wtimer_set_wakeup(&wtimer, 1<<16, me);
    wtimer_set_wakeup(&wtimer2, 1<<16, me);

    wtimer_set_wakeup(&wtimer, 4000000, me);
    wtimer_set_wakeup(&wtimer2, 3000000, me);

    wtimer_set_wakeup(&wtimer, 200000, me);
    wtimer_set_wakeup(&wtimer2, 100000, me);

    printf("now=%u\n", (unsigned)wtimer_now());
    thread_sleep();
    printf("now=%u\n", (unsigned)wtimer_now());
    thread_sleep();
    printf("now=%u\n", (unsigned)wtimer_now());

    printf("Test completed!\n");

    return 0;
}
