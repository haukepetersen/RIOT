/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_ggs_sig BT SIG defined GATT services
 * @ingroup     net_ggs
 * @brief       BT SIG specific GATT services
 * @{
 *
 * @file
 * @brief       Definition of BT SIG specific GATT
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GGS_SERVICE_SIG_H
#define GGS_SERVICE_SIG_H

#include "net/gorm/gatt.h"

#define GGS_UUID_IPS            (0x1820)

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: not sure we ever need this */
// static const gorm_uuid_base_t gorm_uuid_sig_base = {{
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//     0x10, 0x00, 0x80, 0x00,
//     0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb
// }};

/**
 * @brief   Export the `Internet Protocol Support Service`
 */
extern gorm_gatt_service_t ggs_ips_service;

#ifdef __cplusplus
}
#endif

#endif /* GGS_SERVICE_SIG_H */
/** @} */
