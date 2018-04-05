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
 * @brief       Gorm's dedicated iBeacon handling submodule
 * @{
 *
 * @file
 * @brief       Gorm's iBeacon specific interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_IBEACON_H
#define GORM_IBEACON_H

#include "net/gorm.h"

#ifdef __cplusplus
extern "C" {
#endif

void gorm_ibeacon_setup(gorm_uuid_t *uuid, uint16_t major, uint16_t minor,
                        uint8_t txpower, uint32_t adv_interval);

#ifdef __cplusplus
}
#endif

#endif /* GORM_IBEACON_H */
/** @} */
