/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm Gorm, the daddy of Bluetooth
 * @ingroup     net
 * @brief       Gorm, the BLE stack for RIOT
 *
 * TODO more high-level doc
 *
 * @todo        Unify concept of static vs dynamic configuaration values, e.g.
 *              - advertisement intervals (conn vs nonconn)
 * @todo        Address handling: generation of public addresses etc
 * @todo        Use netdev correctly (do proper switching to thread context)
 * @todo        Implement and use `event_post_first()` and use in link layer for
 *              received packets
 * @todo        check for consistent naming of static functions (_x vs x)
 *
 * @{
 * @file
 * @brief       High level interface for telling Gorm how to behave
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_H
#define GORM_H

#include <stdint.h>

#include "clist.h"

#include "net/netdev/ble.h"
#include "net/gorm/config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Maximum link layer PDU length (excluding 2 byte header) [0:255] bytes
 */
#define GORM_PDU_MAXLEN         (255U)

typedef struct gorm_pdu {
    struct gorm_pdu *next;
    netdev_ble_pkt_t pkt;
} gorm_buf_t;

/**
 * @brief    Gorm's error codes
 */
/* TODO: Move to dedicated header?! */
enum {
    GORM_OK     = 0,
    GORM_ERR_TIMINGS = -1,
};


/**
 * @brief   Run Gorm's link layer
 */
void gorm_run_controller(netdev_t *radio);

/**
 * @brief   Initialize Gorm's host implementation
 */
void gorm_host_init(void);

/**
 * @brief   Run Gorm's host layers
 */
void gorm_host_run(void);

#ifdef __cplusplus
}
#endif

#endif /* GORM_H */
/** @} */
