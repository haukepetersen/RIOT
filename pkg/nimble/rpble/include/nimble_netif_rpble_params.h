/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_nimble_netif_rpble
 *
 * @{
 * @file
 * @brief       Default configuration for the nimble_netif_rpble module
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NIMBLE_NETIF_RPBLE_PARAMS_H
#define NIMBLE_NETIF_RPBLE_PARAMS_H

#include "nimble_netif_rpble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Default parameters used for the nimble_netif_rpble module
 * @{
 */
#ifndef NIMBLE_NETIF_RPBLE_SCAN_ITVL
#define NIMBLE_NETIF_RPBLE_SCAN_ITVL          (1000000U)
#endif
#ifndef NIMBLE_NETIF_RPBLE_SCAN_WIN
#define NIMBLE_NETIF_RPBLE_SCAN_WIN           (200000U)
#endif

#ifndef NIMBLE_NETIF_RPBLE_ADV_ITVL
#define NIMBLE_NETIF_RPBLE_ADV_ITVL           (100000U)
#endif

#ifndef NIMBLE_NETIF_RPBLE_CONN_SCANITVL
#define NIMBLE_NETIF_RPBLE_CONN_SCANITVL      (2000000U)
#endif
#ifndef NIMBLE_NETIF_RPBLE_CONN_SCANWIN
#define NIMBLE_NETIF_RPBLE_CONN_SCANWIN       (2000000U)
#endif
#ifndef NIMBLE_NETIF_RPBLE_CONN_ITVL
#define NIMBLE_NETIF_RPBLE_CONN_ITVL          (25000U)
#endif
#ifndef NIMBLE_NETIF_RPBLE_CONN_LATENCY
#define NIMBLE_NETIF_RPBLE_CONN_LATENCY       (0)
#endif
#ifndef NIMBLE_NETIF_RPBLE_CONN_SUPER_TO
#define NIMBLE_NETIF_RPBLE_CONN_SUPER_TO      (1000000U)
#endif
#ifndef NIMBLE_NETIF_RPBLE_CONN_TIMEOUT
#define NIMBLE_NETIF_RPBLE_CONN_TIMEOUT       (2000000U)
#endif
#ifndef NIMBLE_NETIF_RPBLE_EVAL_TIMEOUT
#define NIMBLE_NETIF_RPBLE_EVAL_TIMEOUT       (5000000U)
#endif
#ifndef NIMBLE_NETIF_RPBLE_CLEAR_TIMEOUT
#define NIMBLE_NETIF_RPBLE_CLEAR_TIMEOUT      (30000000U)
#endif
#ifndef NIMBLE_NETIF_RPBLE_NAME
#define NIMBLE_NETIF_RPBLE_NAME              "R"  /* 1 char max */
#endif

#ifndef NIMBLE_NETIF_RPBLE_PARAMSNIMBLE_NETIF_RPBLE_NAME
#define NIMBLE_NETIF_RPBLE_PARAMS                        \
    { .scan_itvl     = NIMBLE_NETIF_RPBLE_SCAN_ITVL,     \
      .scan_win      = NIMBLE_NETIF_RPBLE_SCAN_WIN,      \
      .adv_itvl      = NIMBLE_NETIF_RPBLE_ADV_ITVL,      \
      .conn_scanitvl = NIMBLE_NETIF_RPBLE_CONN_SCANITVL, \
      .conn_scanwin  = NIMBLE_NETIF_RPBLE_CONN_SCANWIN,  \
      .conn_itvl     = NIMBLE_NETIF_RPBLE_CONN_ITVL,     \
      .conn_latency  = NIMBLE_NETIF_RPBLE_CONN_LATENCY,  \
      .conn_super_to = NIMBLE_NETIF_RPBLE_CONN_SUPER_TO, \
      .conn_timeout  = NIMBLE_NETIF_RPBLE_CONN_TIMEOUT,  \
      .eval_timeout  = NIMBLE_NETIF_RPBLE_EVAL_TIMEOUT,  \
      .clear_timeout = NIMBLE_NETIF_RPBLE_CLEAR_TIMEOUT, \
      .name          = NIMBLE_NETIF_RPBLE_NAME }
#endif
/**@}*/

/**
 * @brief   nimble_netif_rpble configuration
 */
static const nimble_netif_rpble_cfg_t nimble_netif_rpble_params =
    NIMBLE_NETIF_RPBLE_PARAMS;

#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_NETIF_RPBLE_PARAMS_H */
/** @} */
