/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_ibeacon Gorm's iBeacon interface
 * @ingroup     net_gorm
 * @brief       Gorm's dedicated iBeacon abstraction
 * @{
 *
 * @file
 * @brief       Gorm's iBeacon abstraction
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_IBEACON_H
#define GORM_IBEACON_H

#include "net/gorm.h"
#include "net/gorm/uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Payload length of iBeacon advertising packets
 */
#define GORM_IBEACON_PDU_LEN        (36U)

typedef struct {
    gorm_ctx_t *ctx;
    uint8_t pdu[GORM_IBEACON_PDU_LEN];
} gorm_ibeacon_ctx_t;

void gorm_ibeacon_advertise(gorm_ibeacon_ctx_t *ctx, gorm_uuid_t *uuid,
                            uint16_t major, uint16_t minor,
                            uint8_t txpower, uint32_t adv_interval);

#ifdef __cplusplus
}
#endif

#endif /* GORM_IBEACON_H */
/** @} */
