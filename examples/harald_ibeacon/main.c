/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       BLE beacon using the Harald BLE stack
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "xtimer.h"
#include "luid.h"
#include "net/harald/ibeacon.h"

#define INTERVAL        (1 * US_PER_SEC)

#define UUID            { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, \
                          0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 }
#define MAJOR           (0x2323)
#define MINOR           (0x4242)
#define TXPOWER         (123U)

int main(void)
{
    uint8_t uuid[] = UUID;

    puts("Harald's beacon example");

    while (1) {
        harald_ibeacon_adv(uuid, MAJOR, MINOR, TXPOWER);
        xtimer_usleep(INTERVAL);
    }

    return 0;
}
