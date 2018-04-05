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

#include "net/gorm/uuid.h"

#include "net/ggs/sig.h"

const gorm_gatt_service_t ggs_ips_service = {
    .uuid = GORM_UUID(GGS_UUID_IPS, NULL),
    .char_cnt = 0,      /* this service has no characteristics */
    .chars = NULL,
};
