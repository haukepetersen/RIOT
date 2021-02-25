/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_gorm
 * @{
 *
 * @file
 * @brief       Gorm's global configuration
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_CONFIG_H
#define GORM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GORM_CFG_CONNECTIONS_NUMOF
#define GORM_CFG_CONNECTIONS_NUMOF                  (4U)
#endif

#ifndef GORM_CFG_LL_PERIPH_ADV_INTERVAL
#define GORM_CFG_LL_PERIPH_ADV_INTERVAL             (1U * US_PER_SEC)
#endif

#ifndef GORM_CFG_LL_PERIPH_ADV_EVENT_DURATION
#define GORM_CFG_LL_PERIPH_ADV_EVENT_DURATION       (10U * US_PER_MS)
#endif

#ifndef GORM_CFG_LL_PERIPH_ADV_CHANNELS
#define GORM_CFG_LL_PERIPH_ADV_CHANNELS             { 37, 38, 39 }
#endif

/* ADVANCED */
/**
 * @brief   Compensate for the time passed between the start of the first packet
 *          received in a connection and the time when the actual callback is
 *          triggered, in [us]
 */
#ifndef GORM_CFG_LL_ANCHOR_OFFSET
#define GORM_CFG_LL_ANCHOR_OFFSET                   (350U)
#endif

/* ADVANCED */
/**
 * @brief   Static window widening value, roughly estimated...
 */
#ifndef GORM_CFG_LL_WIN_WIDENING
#define GORM_CFG_LL_WIN_WIDENING                    (50U)
#endif

/* ADVANCED */
/**
 * @brief   Time to listen for incoming data while in a connection
 *
 * Default time is 300us (2 times T_IFS)
 */
#ifndef GORM_CFG_LL_RX_TIMEOUT
#define GORM_CFG_LL_RX_TIMEOUT                      (300U + GORM_CFG_LL_RX_RAMPUP_DELAY)
#endif


/* TODO: nRF52 specific timing, move to CPU/radio specific config section */
#ifndef GORM_CFG_LL_RX_RAMPUP_DELAY
#define GORM_CFG_LL_RX_RAMPUP_DELAY                 (165U)
#endif


#ifndef GORM_CFG_VERSION
#define GORM_CFG_VERSION                            BLE_VERSION_50
#endif

#ifndef GORM_CFG_VENDOR_ID
#define GORM_CFG_VENDOR_ID                          (0xffff)
#endif

#ifndef GORM_CFG_SUB_VERS_NR
#define GORM_CFG_SUB_VERS_NR                        (0xabcd)
#endif

#ifndef GORM_CFG_POOLSIZE
#define GORM_CFG_POOLSIZE                           (10U)
#endif

/**
 * @brief   Thread priority assigned to the Gorm thread in standalone mode
 *
 * @todo    Move to RIOT specific configuration!
 */
#ifndef GORM_CFG_THREAD_STACKSIZE_HOST
#define GORM_CFG_THREAD_STACKSIZE_HOST              (THREAD_STACKSIZE_DEFAULT)
#endif
#ifndef GORM_CFG_THREAD_PRIO_HOST
#define GORM_CFG_THREAD_PRIO_HOST                   (THREAD_PRIORITY_MAIN - 1)
#endif
#ifndef GORM_CFG_THREAD_NAME_HOST
#define GORM_CFG_THREAD_NAME_HOST                   "gorm_host"
#endif

#ifndef GORM_CFG_THREAD_STACKSIZE_CONTROLLER
#define GORM_CFG_THREAD_STACKSIZE_CONTROLLER        (THREAD_STACKSIZE_DEFAULT)
#endif
#ifndef GORM_CFG_THREAD_PRIO_CONTROLLER
#define GORM_CFG_THREAD_PRIO_CONTROLLER             (0)
#endif
#ifndef GORM_CFG_THREAD_NAME_CONTROLLER
#define GORM_CFG_THREAD_NAME_CONTROLLER             "gorm_ctrl"
#endif

#ifdef __cplusplus
}
#endif

#endif /* GORM_CONFIG_H */
