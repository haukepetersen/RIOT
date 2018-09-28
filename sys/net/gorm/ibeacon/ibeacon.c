/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm_ibeacon
 * @{
 *
 * @file
 * @brief       Gorm's iBeacon implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <string.h>

#include "assert.h"
#include "byteorder.h"

#include "net/netdev/ble.h"
#include "net/gorm/ll.h"
#include "net/gorm/ibeacon.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

/* payload structure used by iBeacons */
typedef struct __attribute__((packed)) {
    uint8_t txadd[BLE_ADDR_LEN];
    uint8_t prefix[9];
    uint8_t uuid[BLE_UUID128_LEN];
    be_uint16_t major;
    be_uint16_t minor;
    uint8_t txpower;
} ibeacon_t;

static const uint8_t _prefix[9] = { 0x02, 0x01, 0x06, 0x1a, 0xff,
                                    0x4c, 0x00, 0x02, 0x15 };

int gorm_ibeacon_advertise(gorm_ibeacon_ctx_t *ibeacon, uint8_t *addr,
                           gorm_uuid_t *uuid,
                           uint16_t major, uint16_t minor,
                           uint8_t txpower, uint32_t adv_interval)
{
    assert(ibeacon && addr && uuid);

    /* configure the iBeacon PDU */
    ibeacon_t *pdu = (ibeacon_t *)ibeacon->gap.adv_data;

    memcpy(pdu->txadd, addr, BLE_ADDR_LEN);
    memcpy(pdu->prefix, _prefix, sizeof(_prefix));
    gorm_uuid_to_buf(pdu->uuid, uuid);
    pdu->major = byteorder_htons(major);
    pdu->minor = byteorder_htons(minor);
    pdu->txpower = txpower;

    /* finish setting up the advertising context */
    ibeacon->gap.adv_len = (uint8_t)sizeof(ibeacon_t);
    ibeacon->gap.interval = adv_interval;

    /* start advertising */
    return gorm_ll_adv_nonconn(&ibeacon->ctx->ll, &ibeacon->gap);
}
