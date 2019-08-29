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
 * @brief       BLE Connection manager that automatically opens BLE
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
 * # To be implemented
 * @todo        filter function could be more powerful, it is probably a good
 *              idea to extend this module to allow for passing custom filter
 *              functions through a function pointer
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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Return codes used by the autoconn module
 */
enum {
    NIMBLE_AUTOCONN_OK      =  0,  /**< like a walk in the park */
    NIMBLE_AUTOCONN_INVALID = -1,  /**< invalid parameters given */
};

typedef struct {
    uint32_t period_scan;           /* in ms */
    uint32_t period_adv;            /* in ms */
    uint32_t period_jitter;         /* in ms */

    uint32_t adv_itvl;              /* in ms */

    /**< scan interval applied while in scanning state [in ms] */
    uint32_t scan_itvl;
    /**< scan window applied while in scanning state [in ms] */
    uint32_t scan_win;

    uint32_t conn_itvl;             /* in ms */
    uint16_t conn_latency;          /* slave latency */
    uint32_t conn_super_to;         /* supervision timeout, in ms */
    /**< Node ID included in the advertising data, may be NULL */
    const char *node_id;
} nimble_autoconn_params_t;

/**
 * @brief   Initialize and enable the autoconn module
 *
 * @note    This function is intended to be called once during system
 *          initialization
 */
void nimble_autoconn_init(void);

/**
 * @brief   Update the used parameters (timing and node ID)
 *
 * @param[in] params        new parameters to apply
 *
 * @return  NIMBLE_AUTOCONN_OK if everything went fine
 * @return  NIMBLE_AUTOCONN_INVALID if given parameters can not be applied
 */
int nimble_autoconn_param_update(const nimble_autoconn_params_t *params);

/**
 * @brief   Enable the automated management of BLE connections
 */
void nimble_autoconn_enable(void);

/**
 * @brief   Disable the automated connection management
 *
 * @note    All existing connections are kept, only the scanning and advertising
 *          is canceled
 */
void nimble_autoconn_disable(void);

#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_AUTOCONN_H */
/** @} */
