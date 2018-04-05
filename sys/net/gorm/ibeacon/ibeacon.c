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

// #inlcude "net/gorm/arch/byteorder.h"
#include "byteorder.h"

#include "net/netdev/ble.h"
#include "net/gorm/ll.h"
#include "net/gorm/ibeacon.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"


typedef struct __attribute__((packed)) {
    uint8_t txadd[6];
    uint8_t prefix[9];
    gorm_uuid_t uuid;
    be_uint16_t major;
    be_uint16_t minor;
    uint8_t txpower;
} gorm_ibeacon_t;

static const uint8_t ibeacon_prefix[9] = { 0x02, 0x01, 0x06, 0x1a, 0xff,
                                           0x4c, 0x00, 0x02, 0x15 };

static netdev_ble_pkt_t pkt;
static gorm_ll_adv_nonconn_t adv;

void gorm_ibeacon_setup(gorm_uuid_t *uuid, uint16_t major, uint16_t minor,
                        uint8_t txpower, uint32_t adv_interval)
{
    /* configure the iBeacon PDU */
    gorm_ibeacon_t *pdu = (gorm_ibeacon_t *)&pkt.pdu;
    pkt.hdr.len = (uint8_t)sizeof(gorm_ibeacon_t);

    memcpy(pdu->txadd, gorm_ll_addr_rand(), GORM_LL_ADDR_LEN);
    memcpy(pdu->prefix, ibeacon_prefix, sizeof(ibeacon_prefix));
    memcpy(&pdu->uuid, uuid, sizeof(gorm_uuid_t));
    pdu->major = byteorder_htons(major);
    pdu->minor = byteorder_htons(minor);
    pdu->txpower = txpower;

    /* embed PDU in BLE packet */
    gorm_ll_adv_nonconn_setup(&adv, &pkt, adv_interval);

    /* and setup the advertiser */
    gorm_ll_adv_nonconn_start(&adv);
}
