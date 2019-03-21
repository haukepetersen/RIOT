/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_facemaster_faceimp_sockudp UDP Sock Face Implementation
 * @ingroup     net_facemaster_faceimp
 * @brief       TODO
 * @{
 *
 * @file
 * @brief       Face implementation for NimBLE
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */



#ifndef NET_FACEIMP_SOCKUDP_H
#define NET_FACEIMP_SOCKUDP_H

#include "net/faceimp.h"
#include "net/sock/udp.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FACEIMP_SOCKUDP_THREAD_PRIO
#define FACEIMP_SOCKUDP_THREAD_PRIO         (THREAD_PRIORITY_MAIN - 5)
#endif

#ifndef FACEIMP_SOCKUDP_STACKSIZE
#define FACEIMP_SOCKUDP_STACKSIZE           (THREAD_STACKSIZE_DEFAULT)
#endif

// TODO: make events faceimp-global?!
typedef enum {
    FACEIMP_154_CONNECTED,
    FACEIMP_154_DISONNECT,
    FACEIMP_154_ABORT,
    FACEIMP_154_SOMETHING_SOMETHING, // TODO...
} faceimp_154_event_t;

typedef struct {
    uint16_t channel;
    uint16_t pan_id;
    // TODO: add more, e.g. CSMA params
} faceimp_154_cfg_t;

typedef struct {
    faceimp_154_t bcast_face;
    netdev_t dev;
    netdev_t *cfg;
    uint8_t addr[FACEIMP_154_ADDRLEN];  /**< address length used is statically configured */
    uint8_t seq_num;
    uint8_t rxbuf[IEEE802154_FRAME_LEN_MAX];
    uint8_t txbuf[IEEE802154_FRAME_LEN_MAX];
    mutex_t txlock;
} faceimp_154_dev_t;

typedef struct {
    faceimp_t imp;
    uint16_t addr;
    faceimp_154_dev_t *impdev;
} faceimp_154_t;


typedef void(*faceimp_154_eventcb_t)(faceimp_154_t *imp154,
                                     faceimp_154_event_t event);

/**
 * @brief   Spawns a thread that is shared by all 15.4 devices
 *
 * @param  [description]
 * @return [description]
 */
/* to be called from: system init (auto_init) */
int faceimp_154_init(void);

/* to be called from: user application or faceman module, possibly also during
 * system init */
int faceimp_154_add_dev(faceimp_154_dev_t *impdev, netdev_t *netdev,
                        const faceimp_154_cfg_t *cfg);

/**
 * @brief   Register a global event callback, servicing all NimBLE Imps
 *
 * @params[in] cb       event callback to register, may be NULL
 */
/* to be called from: user application or faceman module */
int faceimp_nimble_eventcb(faceimp_nimble_eventcb_t cb);

/* to be called from: user application or faceman module */
int faceimp_154_connect(faceimp_154_dev_t *impdev,
                        faceimp_154_t *imp,
                        uint16_t addr);

int faceimp_154_terminate(faceimp_154_t *imp);

#ifdef __cplusplus
}
#endif

#endif /* NET_FACEIMP_154_H */
/** @} */
