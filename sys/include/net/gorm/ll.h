/*
 * Copyright (C) 2017,2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm
 * @{
 *
 * @file
 * @brief       Gorm's BLE link layer
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @todo        Split this header for periph/bcaster/host/...
 */

#ifndef GORM_LL_H
#define GORM_LL_H

#include "mutex.h"
#include "assert.h"
#include "event/timeout.h"
#include "event/callback.h"

#include "net/netdev/ble.h"
#include "net/gorm.h"
#include "net/gorm/pduq.h"
#include "net/gorm/arch/timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Constant value of advertising CRC and access address
 * @{
 */
#define GORM_LL_ADV_AA          (0x8e89bed6)
#define GORM_LL_ADV_CRC         (0x00555555)
/** @} */

/* TODO: moved to ble.h */
/**
 * @name    BLE advertising data packet types
 * @{
 */
#define GORM_LL_PDU_MASK        (0x0f)
#define GORM_LL_ADV_IND         (0x00)
#define GORM_LL_DIRECT_IND      (0x01)
#define GORM_LL_ADV_NONCON_IND  (0x02)
#define GORM_LL_SCAN_REQ        (0x03)
#define GORM_LL_AUX_SCAN_REQ    (0x03)
#define GORM_LL_SCAN_RESP       (0x04)
#define GORM_LL_CONNECT_IND     (0x05)
#define GORM_LL_AUX_CONNECT_REQ (0x05)
#define GORM_LL_ADV_SCAN_IND    (0x06)
#define GORM_LL_ADV_EXT_IND     (0x07)
#define GORM_LL_AUX_ADV_IND     (0x07)
#define GORM_LL_AUX_SCAN_RSP    (0x07)
#define GORM_LL_AUX_SYNC_IND    (0x07)
#define GORM_LL_AUX_CHAIN_IND   (0x07)
#define GORM_LL_CONNECT_RESP    (0x08)
/** @} */

/**
 * @name    Advertising data PDU flags
 */
#define GORM_LL_FLAG_CHSEL      (0x20)
#define GORM_LL_FLAG_TXADD      (0x40)
#define GORM_LL_FLAG_RXADD      (0x80)
/** @} */

/**
 * @name    Link layer data PDU flags
 * @{
 */
#define GORM_LL_LLID_MASK       (0x03)
#define GORM_LL_LLID_DATA_CONT  (0x01)  /**< continued or empty packet */
#define GORM_LL_LLID_DATA_START (0x02)
#define GORM_LL_LLID_CTRL       (0x03)
#define GORM_LL_NESN            (0x04)  /**< next expected sequence number */
#define GORM_LL_SN              (0x08)  /**< sequence number */
#define GORM_LL_MD              (0x10)  /**< more data */
#define GORM_LL_SEQ_MASK        (0x0c)  /**< mask SN and NESN */
#define GORM_LL_FLOW_MASK       (0x1c)  /**< mask SN, NESN, and MD */
/** @} */


/* TODO: re-order fields to optimize memory usage... */
typedef struct  gorm_ll_connection_struct {
    /* connection state: holds ll state, flow control, and anchor state */
    uint8_t state;

    /* context data when in connection state */
    uint16_t event_counter;
    netdev_ble_ctx_t ctx;

    /* channel configuration */
    uint8_t chan_hop;               /**< hop increment value */
    uint8_t chan_map[5];            /**< active channel map */
    uint8_t chan_cnt;               /**< number of active channels */
    uint8_t chan_unmapped;          /**< next unmapped channel */
    /* TODO: add channel selection algorithm as function pointer? */            /**< remapped next channel */

    /* used timers */
    gorm_arch_timer_t timer_spv;
    gorm_arch_timer_t timer_con;

    /* timing parameters */
    uint32_t timeout_spv;       /**< supervision timeout */
    uint32_t timeout_rx;        /**< receive timeout */
    uint32_t interval;          /**< connection event interval */
    uint16_t slave_latency; /**< count number of connection events, reset on new connection */
    // uint32_t adv_interval;   /**< -> we use a static define for this (for now) */

    /* buffered timing parameter update values */
    struct {
        uint16_t instant;       /**< apply the update when event_counter hits this value */
        uint8_t raw[9];
    } timings_update;

    /* buffered channel map update values */
    struct {
        uint16_t instant;
        uint8_t map[5];
        uint8_t cnt;
    } chan_update;

    /* shadow fields for updating channel config and timings */
    /* TODO */

    /* pkt queues for passing data to and receiving data from l2cap layer */
    gorm_pduq_t rxq;
    gorm_pduq_t txq;
    gorm_buf_t *in_tx;
    gorm_buf_t *in_rx;

    /* TODO: additional information needed for triggering events on higher layer? */

    /* GATT specific values */
    /* TODO: split these into separate struct(s) and join structs on higher layer */
    // uint16_t gatt_max_mtu;
} gorm_ll_connection_t;

typedef struct {
    gorm_arch_timer_t timer;
    uint32_t interval;
    netdev_ble_pkt_t *pkt;
} gorm_ll_adv_nonconn_t;

extern event_queue_t gorm_ll_runqueue;
extern event_queue_t gorm_ll_waitqueue;

/* TODO: only include in peripheral, scanner, and central roles */
extern event_callback_t gorm_ll_rx_action;


void gorm_ll_run(netdev_t *dev);

uint8_t *gorm_ll_addr_rand(void);


/* TODO: only include in peripheral, scanner, and central roles */
static inline int gorm_ll_busy(void)
{
    return (gorm_ll_rx_action.callback != NULL);
}


static inline int gorm_ll_get_type(netdev_ble_pkt_t *pkt)
{
    return (int)(pkt->flags & GORM_LL_PDU_MASK);
}


/* TODO: merge broadcaster code with `ll_perihp`, use gorm_ll_connection_t for
 *       this also. */
#if 0
#ifdef MODULE_GORM_BROADCASTER
void gorm_ll_adv_nonconn_setup(gorm_ll_adv_nonconn_t *adv,
                               netdev_ble_pkt_t *pkt, uint32_t interval);

static inline void gorm_ll_adv_nonconn_start(gorm_ll_adv_nonconn_t *adv)
{
    assert(adv);
    gorm_arch_evtim_set_from_now(&adv->timer, adv->interval);
}

static inline void gorm_ll_adv_nonconn_stop(gorm_ll_adv_nonconn_t *adv)
{
    assert(adv);
    gorm_arch_evtim_cancel(&adv->timer);
}
#endif
#endif


#ifdef MODULE_GORM_PERIPHERAL
void gorm_ll_periph_init(void);
void gorm_ll_periph_adv_start(void);
void gorm_ll_periph_adv_setup(void *ad_adv, size_t adv_len,
                              void *ad_scan, size_t scan_len);

/**
 * @brief   Close the given connection, independent of its state
 *
 * If the connection is currently in advertising or connected state, it will
 * be put into STANDBY state. If the connection is already STANDBY, nothing
 * will happen.
 *
 * @param[in,out] con   connection to close
 */
void gorm_ll_periph_terminate(gorm_ll_connection_t *con);

/* TODO: find a better way to pass connections form LL to l2cap, probably turn
 *       the buffer handling around, so that the upper layer passes connection
 *       structs to the LL?! */
gorm_ll_connection_t *gorm_ll_periph_getcon(void);
#endif


#ifdef __cplusplus
}
#endif

#endif /* GORM_LL_H */
/** @} */
