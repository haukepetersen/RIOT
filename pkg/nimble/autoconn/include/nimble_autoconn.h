/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    pkg_nimble_autoconn Simple Connection Manager for Nimble netif
 * @ingroup     pkg_nimble
 * @brief       BLE Connection manager that automatically opens netif
 *              connections to any node that fit the given filter criteria
 *
 * # About
 * TODO
 *
 * # State Machine
 * alternate between:
 * - advertising (accepting incoming connection requests) for `period_adv` + jitter
 * - scanning (actively opening connections) for `period_scan` + jitter
 *
 * @{
 *
 * @file
 * @brief       GNRC netif implementation for NimBLE
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NIMBLE_AUTOCONN_H
#define NIMBLE_AUTOCONN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t period_scan;           /* in ms */
    uint32_t period_adv;            /* in ms */
    uint32_t period_jitter;         /* in ms */

    uint32_t adv_itvl;              /* in ms */

    uint32_t scan_itvl;             /* in ms */
    uint32_t scan_win;              /* in ms */

    uint32_t conn_itvl;             /* in ms */
    uint16_t conn_latency;          /* slave latency */
    uint32_t conn_super_to;         /* supervision timeout, in ms */
    uint32_t conn_timeout;      // TODO merge with scanitv and scanwin
    const char *node_id;
} nimble_autoconn_params_t;

void nimble_autoconn_init(void);

void nimble_autoconn_param_update(const nimble_autoconn_params_t *params);


void nimble_autoconn_enable(void);

void nimble_autoconn_disable(void);

#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_AUTOCONN_H */
/** @} */
