/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_ggs_sig
 * @{
 *
 * @file
 * @brief       Implementation of the BT SIG Internet Protocol Support Service
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "net/ble.h"
#include "net/ggs/sig.h"
#include "net/gorm/uuid.h"

gorm_gatt_service_t ggs_ips_service = {
    .uuid = GORM_UUID(BLE_SERVICE_INTERNET_PROT_SUPPORT, NULL),
    .char_cnt = 0,      /* this service has no characteristics */
    .chars = NULL,
};
