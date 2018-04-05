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
 * @brief       Test some properties of xtimer that are relevant to Gorm
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <stdio.h>

#include "xtimer.h"

#define TEST_INT            (100 * US_PER_MS)
#define REFRESH_INT         (50 * US_PER_MS)
#define REFRESHES           (10U)

static xtimer_t ttest;
static xtimer_t trefresh;

static unsigned cnt = 0;

static void test_cb(void *arg)
{
    (void)arg;
    puts("DELAYED timer fired");
}

static void refresh_cb(void *arg)
{
    (void)arg;
    puts("Refresh timer fired");

    if ((cnt++) < REFRESHES) {
        xtimer_set(&trefresh, REFRESH_INT);
        xtimer_set(&ttest, TEST_INT);
    }
}

int main(void)
{
    puts("Gorm's xtimer test");

    ttest.callback = test_cb;
    trefresh.callback = refresh_cb;

    xtimer_set(&trefresh, REFRESH_INT);
    xtimer_set(&ttest, TEST_INT);
    return 0;
}
