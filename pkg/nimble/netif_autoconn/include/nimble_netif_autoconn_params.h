/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     nimble_netif_autconn
 *
 * @{
 * @file
 * @brief       Default configuration for the nimble_netif_autoconn module
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NIMBLE_NETIF_AUTOCONN_PARAMS_H
#define NIMBLE_NETIF_AUTOCONN_PARAMS_H

#include "nimble_netif_autoconn.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Default parameters used for the nimble_netif_autoconn module
 * @{
 */
#ifndef NIMBLE_NETIF_AUTOCONN_PERIOD_SCAN
#define NIMBLE_NETIF_AUTOCONN_PERIOD_SCAN       (2000000U)
#endif
#ifndef NIMBLE_NETIF_AUTOCONN_PERIOD_ADV
#define NIMBLE_NETIF_AUTOCONN_PERIOD_ADV        (10000000U)
#endif
#ifndef NIMBLE_NETIF_AUTOCONN_PERIOD_JITTER
#define NIMBLE_NETIF_AUTOCONN_PERIOD_JITTER     (1000000U)
#endif

#ifndef NIMBLE_NETIF_AUTOCONN_SCAN_ITVL
#define NIMBLE_NETIF_AUTOCONN_SCAN_ITVL         (1000000U)
#endif
#ifndef NIMBLE_NETIF_AUTOCONN_SCAN_WIN
#define NIMBLE_NETIF_AUTOCONN_SCAN_WIN          (200000U)
#endif

#ifndef NIMBLE_NETIF_AUTOCONN_ADV_ITVL
#define NIMBLE_NETIF_AUTOCONN_ADV_ITVL          (100000U)
#endif

#ifndef NIMBLE_NETIF_AUTOCONN_CONN_SCANITVL
#define NIMBLE_NETIF_AUTOCONN_CONN_SCANITVL     (2000000U)
#endif
#ifndef NIMBLE_NETIF_AUTOCONN_CONN_SCANWIN
#define NIMBLE_NETIF_AUTOCONN_CONN_SCANWIN      (2000000U)
#endif
#ifndef NIMBLE_NETIF_AUTOCONN_CONN_ITVL
#define NIMBLE_NETIF_AUTOCONN_CONN_ITVL         (25000U)
#endif
#ifndef NIMBLE_NETIF_AUTOCONN_CONN_LATENCY
#define NIMBLE_NETIF_AUTOCONN_CONN_LATENCY      (0)
#endif
#ifndef NIMBLE_NETIF_AUTOCONN_CONN_SUPER_TO
#define NIMBLE_NETIF_AUTOCONN_CONN_SUPER_TO     (1000000U)
#endif
#ifndef NIMBLE_NETIF_AUTOCONN_CONN_TIMEOUT
#define NIMBLE_NETIF_AUTOCONN_CONN_TIMEOUT      (2000000U)
#endif
#ifndef NIMBLE_NETIF_AUTOCONN_NAME
#define NIMBLE_NETIF_AUTOCONN_NAME              "RIOT-autoconn"
#endif

#ifndef NIMBLE_NETIF_AUTOCONN_PARAMS
#define NIMBLE_NETIF_AUTOCONN_PARAMS                        \
    { .period_scan   = NIMBLE_NETIF_AUTOCONN_PERIOD_SCAN,   \
      .period_adv    = NIMBLE_NETIF_AUTOCONN_PERIOD_ADV,    \
      .period_jitter = NIMBLE_NETIF_AUTOCONN_PERIOD_JITTER, \
      .scan_itvl     = NIMBLE_NETIF_AUTOCONN_SCAN_ITVL,     \
      .scan_win      = NIMBLE_NETIF_AUTOCONN_SCAN_WIN,      \
      .adv_itvl      = NIMBLE_NETIF_AUTOCONN_ADV_ITVL,      \
      .conn_scanitvl = NIMBLE_NETIF_AUTOCONN_CONN_SCANITVL, \
      .conn_scanwin  = NIMBLE_NETIF_AUTOCONN_CONN_SCANWIN,  \
      .conn_itvl     = NIMBLE_NETIF_AUTOCONN_CONN_ITVL,     \
      .conn_latency  = NIMBLE_NETIF_AUTOCONN_CONN_LATENCY,  \
      .conn_super_to = NIMBLE_NETIF_AUTOCONN_CONN_SUPER_TO, \
      .conn_timeout  = NIMBLE_NETIF_AUTOCONN_CONN_TIMEOUT,  \
      .name          = NIMBLE_NETIF_AUTOCONN_NAME }
#endif
/**@}*/

/**
 * @brief   nimble_netif_autoconn configuration
 */
static const nimble_netif_autoconn_cfg_t nimble_netif_autoconn_params =
    NIMBLE_NETIF_AUTOCONN_PARAMS;

#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_NETIF_AUTOCONN_PARAMS_H */
/** @} */
