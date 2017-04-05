/*
 * Copyright (C) 2017 Freie Universität Berlin
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
 * @brief       Test the basic concepts of tim
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <time.h>

#include "tim.h"

static void event(void *arg)
{
    int num = (int)arg;
    printf("BURST TIMEOUT: %i\n", num);
}

static void cont_evt(void *arg)
{
    printf("CONT TIMEOUT: %i\n", (int)arg);
}

static void moin(void *arg)
{
    (void)arg;
    puts("moin");
}

static tim_base_t cbs[20];
static int order[] = { 1, 5, 7, 9, 2, 3, 4, 6, 8 , 10 };

int main(void)
{
    tim_init();

    puts("Hello tim");
    tim_sleep(US_BURST, 1000000);
    puts("Hello Hauke");
    tim_sleep(MS_CONT, 1024);
    puts("Hello again.");

    for (int i = 0; i < 5; i++) {
        tim_set_cb(MS_CONT, 512, &cbs[i], moin, NULL);
    }

    tim_sleep(US_BURST, 1000000);

    for (int i = 0; i < 10; i++) {
        tim_set_cb(US_BURST, (1000000 * order[i]), &cbs[i], event, (void *)i);
    }
    puts("10 cb timers set -> BURST");

    for (int i = 10; i < 20; i++) {
        tim_set_cb(MS_CONT, (10240 + (1024 * order[i - 10])), &cbs[i], cont_evt, (void *)i);
    }
    puts("10 cb timers set -> CONT");

}
