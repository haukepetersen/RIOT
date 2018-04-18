/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_ggs_riot RIOT GATT services for Gorm
 * @ingroup     net_ggs
 * @brief       RIOT specific GATT services to be used with Gorm
 * @{
 *
 * @file
 * @brief       Definition of RIOT specific GATT defines for Gorm
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GGS_SERVICE_RIOT_H
#define GGS_SERVICE_RIOT_H

#include "net/gorm/gatt.h"
#include "net/gorm/uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GORM_UUID_RIOT_LED_SERVICE      (0x0001)
#define GORM_UUID_RIOT_LED_CHAR         (0x0002)

static const gorm_uuid_base_t ggs_uuid_riot_base = {{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0a, 0x0f, 0x0f, 0x0e,
    0x00, 0x00, 0x52, 0x49, 0x4f, 0x54
}};

/**
 * @brief   Export the RIOT LED service
 */
extern gorm_gatt_service_t ggs_riot_led_service;

#ifdef __cplusplus
}
#endif

#endif /* GGS_SERVICE_RIOT_H */
/** @} */
