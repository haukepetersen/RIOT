/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       List for keeping scanned BLE devices
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef SCANLIST_H
#define SCANLIST_H

#include "clist.h"
#include "net/ble.h"
#include "nimble/ble.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SCANLIST_SIZE
#define SCANLIST_SIZE               (20U)
#endif

#define SCANLIST_NAME_MAXLEN        (35U)

typedef struct {
    clist_node_t node;
    ble_addr_t addr;
    uint8_t ad[BLE_ADV_PDU_LEN];
    uint8_t ad_len;
    uint32_t adv_msg_cnt;
    uint32_t first_update;
    uint32_t last_update;
} scanlist_t;

void scanlist_init(void);

void scanlist_update(const ble_addr_t *addr, const uint8_t *ad, size_t len);

void scanlist_clear(void);

void scanlist_print(void);

#ifdef __cplusplus
}
#endif

#endif /* SCANLIST_H */
/** @} */
