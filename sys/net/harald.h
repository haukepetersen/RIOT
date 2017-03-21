/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_harald Harald, the RIOT BLE stack
 * @ingroup     sys_net
 * @brief       Harald, the BLE stack for RIOT
 *
 * Restrictions:
 * - support for a single BLE device only
 *
 * Modules:
 * - controller
 *   - physical layer           channel hopping
 *   - link layer               states (standby, adv, scanning, initiating,
 *                              connection states)
 * - host
 *   - L2CAP                    logical link control and adaption profile
 *   - SM                       security manager
 *   - ATT                      attribute protocol
 *   - GATT                     generic attribute profile
 *   - GAP                      generic access profile
 *   - HCI                      host controller interface
 *
 *
 * TODO:
 * - initialization concept:    figure out how to see if device driver exists,
 *                              do ble device initialization somehow
 * - IPSP                       Internet Protocol Support Profile
 *
 * @{
 * @file
 * @brief       High level interface for telling Harald how to behave
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_HARALD_H
#define NET_HARALD_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif
/** @} */
