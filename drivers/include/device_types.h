/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_driver
 *
 * @{
 *
 * @file
 * @brief       Driver device type definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef DEVICE_TYPES_H
#define DEVICE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif




#define DEVICE_FAM_MASK                 (0xff000000)
#define DEVICE_GROUP_MASK               (0x00ff0000)
#define DEVICE_MODEL_MASK               (0x0000ffff)


/**
 * @name    Definition of device families
 * @{
 */
#define DEVICE_NET                      (1 << 24)
#define DEVICE_ACT                      (2 << 24)
#define DEVICE_SENSE                    (3 << 24)
#define DEVICE_STORAGE                  (4 << 24)
/** @} */

/**
 * @name    Definition of device groups
 * @{
 */
/* network device groups */
#define DEVICE_NET_IEEE802154           (DEV_NET | (1 << 16))
#define DEVICE_NET_ETH                  (DEV_NET | (1 << 16))
#define DEVICE_NET_WIFI                 (DEV_NET | (3 << 16))
/* actuator device groups */
#define DEVICE_ACT_LED_RGB              (DEVICE_ACT | (1 << 16))
#define DEVICE_ACT_SERVO                (DEVICE_ACT | (2 << 16))
#define DEVICE_ACT_MOTOR                (DEVICE_ACT | (1 << 16))
#define DEVICE_ACT_SWITCH               (DEVICE_ACT | (4 << 16))
/* sensor device groups */
#define DEVICE_SENSE_BTN                (DEVICE_SENSE | (1 << 16))
#define DEVICE_SENSE_TEMP               (DEVICE_SENSE | (2 << 16))
#define DEVICE_SENSE_HUM                (DEVICE_SENSE | (3 << 16))
#define DEVICE_SENSE_LIGHT              (DEVICE_SENSE | (4 << 16))
/** @} */

/**
 * @name    Definition of device models
 * @{
 */
/* IEEE802.15.4 based models */
#define DEVICE_NET_IEEE802154_AT86RF2XX     (DEVICE_NET_IEEE802154 | 1)
/* Ethernet based models */
#define DEVICE_NET_ETH_W5100                (DEVICE_NET_ETH | 1)
#define DEVICE_NET_ETH_ENC28J60             (DEVICE_NET_ETH | 2)
#define DEVICE_NET_ETH_ENCX24J600           (DEVICE_NET_ETH | 3)
    /* ... extend as needed ... */
    /* ... */
#define DEVICE_SENSE_BTN_GPIO               (DEVICE_SENSE_BTN | 1)
    /* ... extend as needed */
    /* ... */
#define DEVICE_ACT_MOTOR_GENERIC            (DEVICE_ACT_MOTOR | 1)
#define DEVICE_ACT_SWITCH_GPIO              (DEVICE_ACT_SWITCH | 1)
    /* ... extend as needed ... */
    /* ... */
    /* ... extend as needed ... */
/** @} */


#ifdef __cplusplus
}
#endif

#endif /* DEVICE_TYPES_H */
/** @} */
