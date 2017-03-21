/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     harald_ibeacon
 * @{
 *
 * @file
 * @brief       Harald's iBeacon implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <string.h>

#include "net/harald.h"
#include "net/harald/ibeacon.h"

#define POS_UUID        (9)
#define POS_MAJOR       (26)
#define POS_MINOR       (28)
#define POS_TXPOWER     (30)

#define UUID_LEN        (16)

#define PREFIX          { 0x02, 0x01, 0x06, 0x1a, 0xff, 0x00, 0x4c, 0x02, 0x15 }


void harald_ibeacon_adv(uint8_t *uuid, uint16_t major, uint16_t minor,
                        uint8_t txpower)
{
    assert(uuid);

    uint8_t pdu[31] = PREFIX;

    memcpy(&pdu[POS_UUID], uuid, UUID_LEN);
    harald_util_set_u16(&pdu[POS_MAJOR], major);
    harald_util_set_u16(&pdu[POS_MINOR], minor);
    pdu[POS_TXPOWER] = txpower;

    harald_l2cap_adv(pdu);
}
