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
 *              connections to any node that fits the given filter criteria
 *
 * # About
 * This submodule implements a connection manager for BLE. It takes care of
 * scanning, advertising, and opening connections to neighboring nodes.For this,
 * autoconn periodically switches between advertising and scanning mode, hence
 * from accepting incoming connection requests to scanning actively for
 * new neighbors.
 *
 * # State Machine
 *
 *
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

/**
 * @brief   Set of configuration parameters needed to run autoconn
 */
typedef struct {
    /**< amount of time spend in scanning mode [in ms] */
    uint32_t period_scan;           /*   in ms */
    /**< amount of time spend in advertising mode [in ms] */
    uint32_t period_adv;            /* in ms */
    /**< a random value from 0 to this value is added to the duration of each
     *   scanning and advertising period [in ms] */
    uint32_t period_jitter;         /* in ms */
    /**< advertising interval used when in advertising mode [in ms] */
    uint32_t adv_itvl;              /* in ms */
    /**< scan interval applied while in scanning state [in ms] */
    uint32_t scan_itvl;
    /**< scan window applied while in scanning state [in ms] */
    uint32_t scan_win;
    /**< connection interval used when opening a new connection [in ms] */
    uint32_t conn_itvl;
    /**< slave latency used for new connections [in ms] */
    uint16_t conn_latency;
    /**< supervision timeout used for new connections [in ms] */
    uint32_t conn_super_to;
    /**< node ID included in the advertising data, may be NULL */
    const char *node_id;
} nimble_autoconn_params_t;

// TODO: make filter configurable
typedef void(*nimble_autoconn_filter_t)(const ble_addr_t *addr, int8_t rssi,
                                        const uint8_t *ad_buf, size_t ad_len);

/**
 * @brief   Initialize and enable the autoconn module
 *
 * @note    This function is intended to be called once during system
 *          initialization
 */
int nimble_autoconn_init(const nimble_autoconn_params_t *params);

/**
 * @brief   Update the used parameters (timing and node ID)
 *
 * @param[in] params        new parameters to apply
 *
 * @return  NIMBLE_AUTOCONN_OK if everything went fine
 * @return  NIMBLE_AUTOCONN_INVALID if given parameters can not be applied
 */
int nimble_autoconn_update(const nimble_autoconn_params_t *params);

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
