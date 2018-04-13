/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
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
 * @brief       BLE iBeacon example using Gorm
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "timex.h"
#include "net/gorm/ibeacon.h"

#define UUID            { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, \
                          0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 }
#define MAJOR           (0x0001)
#define MINOR           (0x0017)
#define TXPOWER         (0U)
#define INTERVAL        (1 * US_PER_SEC)

static gorm_ibeacon_ctx_t _ctx;

int main(void)
{
    puts("Gorm's iBeacon example");

    /* configure the iBeacon and start advertising it */
    gorm_uuid_t uuid = {UUID};
    gorm_ibeacon_advertise(&_ctx, &uuid, MAJOR, MINOR, TXPOWER, INTERVAL);

    return 0;
}
