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
#include "net/ggs/riot.h"
#include "net/gorm/ibeacon.h"
#include "net/gorm/addr.h"

#define UUID            (0xaffe)
#define MAJOR           (0x0001)
#define MINOR           (0x0017)
#define TXPOWER         (0U)
#define INTERVAL        (1 * US_PER_SEC)

static gorm_ibeacon_ctx_t _ctx;

int main(void)
{
    puts("Gorm's iBeacon example");

    /* we are using the RIOT base UUID */
    gorm_uuid_t uuid = GORM_UUID(UUID, &ggs_riot_uuid_base);

    /* generate a random device address */
    uint8_t addr[BLE_ADDR_LEN];
    gorm_addr_gen_random(addr);

    /* configure the iBeacon and start advertising it */
    int ret = gorm_ibeacon_advertise(&_ctx, addr, &uuid,
                                     MAJOR, MINOR, TXPOWER, INTERVAL);

    if (ret == GORM_OK) {
        puts("iBeacon is now being advertised\n");
    }
    else {
        puts("error: unable to advertise iBeacon\n");
    }

    return 0;
}
