/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_blueface Blueface
 * @ingroup     net
 * @brief       Abstraction API for BLE data streams
 * @{
 *
 * @file
 * @brief       API definition for BLE-based data streams
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef BLUEFACE_H
#define BLUEFACE_H

#include <stdint.h>
#include <stddef.h>

#include "clist.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    BLUEFACE_OK     =  0,
    BLUEFACE_DEVERR = -1,
};

enum {
    BLUEFACE_EVT_CONNECTED      = (1 << 0),
    BLUEFACE_EVT_DISCONNECTED   = (1 << 1),
    BLUEFACE_EVT_ABORT          = (1 << 2),
    BLUEFACE_EVT_CONN_UPDATED   = (1 << 3),
};

typedef struct blueface blueface_t;

typedef void(*blueface_evt_cb_t)(blueface_t *face, uint16_t evt);
typedef void(*blueface_data_cb_t)(blueface_t *face, void *ctx);

struct blueface {
    clist_node_t node;      /**< list entry */
    void *connection;
};

typedef struct {
    blueface_evt_cb_t evt_cb;
    uint16_t events;
    blueface_data_cb_t data_cb;
    uint16_t cid;
} blueface_cfg_t;

int blueface_init(const blueface_cfg_t *cfg);

int blueface_connect()


#ifdef __cplusplus
}
#endif

#endif /* BLUEFACE_H */
/** @} */
