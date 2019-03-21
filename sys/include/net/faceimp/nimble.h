/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_facemaster_faceimp_nimble Nimble Imp
 * @ingroup     net_facemaster_faceimp
 * @brief       TODO
 *
 * TODO:
 * - how to prevent 'deadlock' if all connection slots are taken by incoming
 *   connections and we can not initiate any outgoing connection anymore? And
 *   is this actually always a problem or only with RPL/HOP -> no parent...
 *
 * @{
 *
 * @file
 * @brief       Face implementation for NimBLE
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */



#ifndef NET_FACEIMP_NIMBLE_H
#define NET_FACEIMP_NIMBLE_H

#include "net/faceimp.h"
#include "host/ble_hs.h"
#include "host/ble_l2cap.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FACEIMP_NIMBLE_MAXMTU
#define FACEIMP_NIMBLE_MAXMTU           (1024U)
#endif

typedef enum {
    FACEIMP_NIMBLE_CONNECTED,
    FACEIMP_NIMBLE_DISCONNECTED,
    FACEIMP_NIMBLE_ABORT,
    FACEIMP_NIMBLE_CONN_UPDATED,
} faceimp_nimble_event_t;

typedef struct {
    const struct ble_gap_conn_params conn_params;
} faceimp_nimble_cfg_t;

typedef struct faceimp_nimble faceimp_nimble_t;

typedef int(*faceimp_nimble_gap_connect_t)(faceimp_nimble_t *nim_imp,
                                           struct ble_gap_event *event);

typedef void(*faceimp_nimble_eventcb_t)(faceimp_nimble_t *nim_imp,
                                        faceimp_nimble_event_t event);

struct faceimp_nimble {
    faceimp_t imp;
    struct ble_l2cap_chan *l2cap_coc;
    uint16_t conn_handle;
};

/* to be called from: system init (auto_init) */
int faceimp_nimble_init(void);

/**
 * @brief   Register a global event callback, servicing all NimBLE Imps
 *
 * @params[in] cb       event callback to register, may be NULL
 */
/* to be called from: user application or faceman module */
int faceimp_nimble_eventcb(faceimp_nimble_eventcb_t cb);

/* to be called from: user application or faceman module */
int faceimp_nimble_connect(faceimp_nimble_t *nim_imp, ble_addr_t *addr,
                           const struct ble_gap_conn_params *conn_params,
                           uint32_t connect_timeout);

int faceimp_nimble_update(faceimp_nimble_t *nim_imp,
                          struct ble_gap_conn_params *conn_params);

/**
 * @brief
 *
 * @warning This function **must** be called from a thread with lower priority
 *          then the thread running NimBLE's host
 *
 * @param nim_imp
 * @param cfg
 *
 * @return
 */
int faceimp_nimble_accept(faceimp_nimble_t *nim_imp,
                          void *ad, size_t ad_len,
                          const struct ble_gap_adv_params *adv_params);

int faceimp_nimble_terminate(faceimp_nimble_t *nim_imp);

#ifdef __cplusplus
}
#endif

#endif /* NET_FACEIMP_NIMBLE_H */
/** @} */
