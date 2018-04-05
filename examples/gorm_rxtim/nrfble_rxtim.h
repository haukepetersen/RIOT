/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_nrf5x_nrfble nRF BLE radio driver
 * @ingroup     drivers_netdev
 * @brief       Radio driver for nRF5x SoCs for using the radio in BLE mode
 *
 * @note        This driver is not thread safe (as of now)
 *

 * @todo        add support for netdev_ble_pkt_ext_t
 * @todo        figure out how to synchronize send and receive and how to make
 *              them thread safe
 *
 * @{
 *
 * @file
 * @brief       Interface definition for the nrfble radio driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NRFBLE_H
#define NRFBLE_H

#include "net/netdev.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   nrfble channel configuration
 * @{
 */
#define NRFBLE_CHAN_MIN             (0U)
#define NRFBLE_CHAN_MAX             (39U)
/** @} */

/**
 * @brief   Default transmission power used
 */
#define NRFBLE_TXPOWER_DEFAULT      (0)                 /* 0dBm */

/**
 * @brief   Export the netdev device descriptor
 */
extern netdev_t nrfble_dev;

/**
 * @brief   Reference to the netdev driver interface
 */
extern const netdev_driver_t nrfble_netdev;

/**
 * @brief   Setup the device driver's data structures
 *
 * @return  pointer to the device's netdev struct
 */
netdev_t *nrfble_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* NRFBLE_H */
/** @} */
