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
 * @brief       This test will print the size of Gorm's main data structures
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <stdio.h>

#include "xtimer.h"
#include "event.h"
#include "event/timeout.h"
#include "event/callback.h"

#include "net/gorm/ll.h"
#include "net/gorm/arch/evtim.h"


int main(void)
{
    puts("Gorm's code size monitoring");
    puts("This test prints the size of Gorm's main data structures. The"
         "results are useful for tracking the regression of Gorm's memory"
         "requirements");

    /* link layer data structures */
    puts("\n--- gorm/ll ---");
    printf("%25s - %u bytes\n",
           "gorm_ll_connection_t", sizeof(gorm_ll_connection_t));
    printf("%25s - %u bytes\n",
           "gorm_ll_adv_nonconn_t", sizeof(gorm_ll_adv_nonconn_t));

    puts("\n--- selected RIOT modules ---");
    printf("%25s - %u bytes\n",
           "xtimer_t", sizeof(xtimer_t));
    printf("%25s - %u bytes\n",
           "event_t", sizeof(event_t));
    printf("%25s - %u bytes\n",
           "event_timeout_t", sizeof(event_timeout_t));
    printf("%25s - %u bytes\n",
           "event_callback_t", sizeof(event_callback_t));
    printf("%25s - %u bytes\n",
           "callback + timeout", sizeof(event_callback_t) + sizeof(event_timeout_t));


    puts("\n[SUCCESS]");
    return 0;
}
