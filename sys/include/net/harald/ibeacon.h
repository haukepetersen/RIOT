/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    harald_ibeacon Harald's iBeacon Interface
 * @ingroup     net_harald
 * @brief       BLE IBeacon interface for Harald
 *
 * Interface for sending BLE beacons.
 *
 * @{
 * @file
 * @brief       High level interface for telling Harald to send Apple IBeacons
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_HARALD_H
#define NET_HARALD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void harald_ibeacon_adv(uint8_t *uuid, uint16_t major, uint16_t minor,
                        uint8_t txpower);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
