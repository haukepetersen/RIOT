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

#ifndef NIMBLE_RPBLE_PARAMS_H
#define NIMBLE_RPBLE_PARAMS_H

#include "nimble_rpble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Default parameters used for the nimble_netif_rpble module
 * @{
 */
#ifndef NIMBLE_RPBLE_SCAN_ITVL
#define NIMBLE_RPBLE_SCAN_ITVL          (1000000U)  /* 1s */
#endif
#ifndef NIMBLE_RPBLE_SCAN_WIN
#define NIMBLE_RPBLE_SCAN_WIN           (120000U)   /* 120ms */
#endif

#ifndef NIMBLE_RPBLE_ADV_ITVL
#define NIMBLE_RPBLE_ADV_ITVL           (100000U)   /* 100ms */
#endif

#ifndef NIMBLE_RPBLE_CONN_SCANITVL
#define NIMBLE_RPBLE_CONN_SCANITVL      (120000U)   /* 120ms */
#endif
#ifndef NIMBLE_RPBLE_CONN_ITVL
#define NIMBLE_RPBLE_CONN_ITVL          (90000U)    /* 90ms */
#endif
#ifndef NIMBLE_RPBLE_CONN_LATENCY
#define NIMBLE_RPBLE_CONN_LATENCY       (0)
#endif
#ifndef NIMBLE_RPBLE_CONN_SUPER_TO
#define NIMBLE_RPBLE_CONN_SUPER_TO      (2000000U)
#endif
#ifndef NIMBLE_RPBLE_CONN_TIMEOUT
#define NIMBLE_RPBLE_CONN_TIMEOUT       (1000000U)  /* 1s */
#endif
#ifndef NIMBLE_RPBLE_EVAL_ITVL
#define NIMBLE_RPBLE_EVAL_ITVL          (10000000U) /* 10s */
#endif
#ifndef NIMBLE_RPBLE_NAME
#define NIMBLE_RPBLE_NAME               "R"  /* 1 char max */
#endif

#ifndef NIMBLE_RPBLE_PARAMS
#define NIMBLE_RPBLE_PARAMS                        \
    { .scan_itvl     = NIMBLE_RPBLE_SCAN_ITVL,     \
      .scan_win      = NIMBLE_RPBLE_SCAN_WIN,      \
      .adv_itvl      = NIMBLE_RPBLE_ADV_ITVL,      \
      .conn_scanitvl = NIMBLE_RPBLE_CONN_SCANITVL, \
      .conn_itvl     = NIMBLE_RPBLE_CONN_ITVL,     \
      .conn_latency  = NIMBLE_RPBLE_CONN_LATENCY,  \
      .conn_super_to = NIMBLE_RPBLE_CONN_SUPER_TO, \
      .conn_timeout  = NIMBLE_RPBLE_CONN_TIMEOUT,  \
      .eval_itvl     = NIMBLE_RPBLE_EVAL_ITVL,     \
      .name          = NIMBLE_RPBLE_NAME }
#endif
/**@}*/

/**
 * @brief   nimble_netif_rpble configuration
 */
static const nimble_rpble_cfg_t nimble_rpble_params =
    NIMBLE_RPBLE_PARAMS;

#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_RPBLE_PARAMS_H */
/** @} */
