/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm
 * @{
 *
 * @file
 * @brief       Gorm's buffer management
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_BUF_H
#define NET_GORM_BUF_H

#include "net/netdev/ble.h"

/**
 * @brief   Generic packet buffer structure used throughout Gorm
 */
typedef struct gorm_pdu {
    struct gorm_pdu *next;
    netdev_ble_pkt_t pkt;
} gorm_buf_t;

#endif /* NET_GORM_BUF_H */
/** @} */
