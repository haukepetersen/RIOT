/*
 * Copyright (C) 2020 Freie Universität Berlin
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
 * @brief       Test for the dbgpin pseudomodule
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "xtimer.h"

#define TICK        (5 * US_PER_MS)     /* -> 5ms */

int main(void)
{
    printf("Found %i configured debug pins\n", dbgpin_cnt());

    for (unsigned p = 0; p < dbgpin_cnt(); p++) {
        printf("Testing pin %u\n", p);
        dbgpin_set(p);
        xtimer_usleep(TICK);
        dbgpin_clr(p);
        xtimer_usleep(TICK);
        dbgpin_tgl(p);
        xtimer_usleep(2 * TICK);
        dbgpin_tgl(p);
        xtimer_usleep(TICK);
        dbgpin_sig(p);
        xtimer_usleep(TICK);
        dbgpin_sig(p);
    }

    puts("Test successful.");

    return 0;
}
