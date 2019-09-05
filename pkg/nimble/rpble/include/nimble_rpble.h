/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_ble_nimble_netif_rpble RPL-over-BLE for NimBLE
 * @ingroup     net_ble_nimble
 * @brief       RPL-over-BLE for Nimble implementation
 *
 * # About
 * This module provides a BLE connection manager for establishing IP-over-BLE
 * connections between BLE nodes based on information provided by the RPL
 * routing protocol.
 *
 * # Strategy
 * - parents are advertising their presence (if slots are open)
 * - children are scanning for potential parents
 * - children will initiate connection procedure 'best' parent
 * - metric for best parent:
 *   - lowest rank
 *   - rssi?
 *    -most free connection slots
 *
 * # State Machine
 * initial: is master?
 * -> yes: start accepting (advertising)
 * -> no: start discovery loop
 *
 * new connection to parent established?
 * -> stop scanning
 * -> start accepting (advertising)
 *
 * # Architecture
 * TODO
 *
 * # TODOs
 * @todo        remove thread and run events solely using NimBLEs event loop
 * @todo        never remove active parents from ppt
 *
 * @{
 *
 * @file
 * @brief       Interface for the generic BLE advertising data processing module
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */


#ifndef NIMBLE_NETIF_RPBLE_H
#define NIMBLE_NETIF_RPBLE_H


#ifdef __cplusplus
extern "C" {
#endif

#ifndef NIMBLE_NETIF_RPBLE_MAXPARENTS
#define NIMBLE_NETIF_RPBLE_MAXPARENTS       (1U)
#endif

#ifndef NIMBLE_NETIF_RPBLE_PPTSIZE
#define NIMBLE_NETIF_RPBLE_PPTSIZE          (5U)
#endif

enum {
    NIMBLE_NETIF_RPBLE_OK        =  0,
    NIMBLE_NETIF_RPBLE_STACKERR  = -1,
    NIMBLE_NETIF_RPBLE_ERR_ADV   = -2,
    NIMBLE_NETIF_RPBLE_NO_CHANGE = -3,
};

typedef struct {
    uint32_t period_scan;
    uint32_t period_adv;
    uint32_t period_jitter;

    uint32_t scan_itvl;
    uint32_t scan_win;

    uint32_t adv_itvl;

    uint32_t conn_scanitvl;
    uint32_t conn_scanwin;      // merge...
    uint32_t conn_itvl;
    uint16_t conn_latency;
    uint32_t conn_super_to;
    uint32_t conn_timeout;      // merge with scanitv and scanwin

    uint32_t eval_timeout;
    uint32_t clear_timeout;

    const char *name;
} nimble_netif_rpble_cfg_t;

typedef struct {
    uint8_t inst_id;
    uint8_t dodag_id[16];
    uint8_t version;
    uint8_t role;
    uint16_t rank;
    uint8_t free_slots;
} nimble_netif_rpble_ctx_t;

int nimble_netif_rpble_init(const nimble_netif_rpble_cfg_t *cfg);

int nimble_netif_rpble_update(const nimble_netif_rpble_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_NETIF_RPBLE_H */
/** @} */
