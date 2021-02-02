/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    pkg_nimble_autoconn Autoconn
 * @ingroup     pkg_nimble
 * @brief       Simple connection manager that automatically opens BLE
 *              connections to any node that fits some given filter criteria
 *
 * @experimental
 *
 * # WARNING
 * This module is highly experimental! Expect bugs, instabilities and sudden API
 * changes :-)
 *
 *
 * # About
 *
 * @{
 *
 * @file
 * @brief       Simple automated connection manager for NimBLE netif
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NIMBLE_AUTOCONN_H
#define NIMBLE_AUTOCONN_H

#include <stdint.h>

#include "nimble_netif.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NIMBLE_AUTOCONN2_FILTER = { 0xff, 0xfe, 0xfa, 0x01, 0x02, 0x03, 0x04 };

typedef struct {
#if IS_ACTIVE(MODULE_NIMBLE_AUTOCONN2_HUB)
    uint32_t scan_itvl_ms;
    uint32_t scan_win_ms;
    uint32_t conn_timeout;
#endif
#if IS_ACTIVE(MODULE_NIMBLE_AUTOCONN2_LEAF)
    uint32_t adv_itvl_ms;
    uint32_t conn_itvl;
    uint32_t conn_latency;
    uint32_t conn_svsn_to;
#endif
} nibmle_autoconn2_params_t;



int nimble_autoconn2_init(nimble_autoconn2_params_t);


int nimble_autoconn2_eventcb(nimble_netif_eventcb_t cb, void *arg);

int nimble_autoconn2_enable(void);

int nimble_autoconn2_disable(void);


#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_AUTOCONN_H */
/** @} */
