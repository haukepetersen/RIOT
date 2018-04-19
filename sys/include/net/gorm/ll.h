/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm_ll
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

#include "net/netdev/ble.h"

#include "net/gorm/buf.h"
#include "net/gorm/arch/timer.h"
#include "net/gorm/gap.h"

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

/**
 * @name    Possible link layer states
 * @{
 */
#define GORM_LL_STATE_ADV       (0x01)
#define GORM_LL_STATE_INIT      (0x02)
#define GORM_LL_STATE_CONN      (0x03)
#define GORM_LL_STATE_STANDBY   (0x04)
#define GORM_LL_STATE_SCAN      (0x05)
#define GORM_LL_STATE_ADV_ONLY  (0x06)
/** @} */

typedef struct  gorm_ll_connection_struct {
    /* connection state: holds ll state, flow control, and anchor state */
    uint8_t state;
    uint8_t flags;

    /* context data when in connection state */
    uint16_t event_counter;
    netdev_ble_ctx_t ctx;

    /* pointer to advertising configuration and data */
    const gorm_gap_adv_ctx_t *gap;

    /* remember address of connected peer */
    uint8_t peer_addr[BLE_ADDR_LEN];

    /* channel configuration */
    uint8_t chan_hop;               /**< hop increment value */
    uint8_t chan_map[5];            /**< active channel map */
    uint8_t chan_cnt;               /**< number of active channels */
    uint8_t chan_unmapped;          /**< next unmapped channel */
    /* TODO: add channel selection algorithm as function pointer */

    /* used timers */
    gorm_arch_timer_t timer_spv;
    gorm_arch_timer_t timer_con;

    /* timing parameters */
    uint32_t timeout_spv;       /**< supervision timeout */
    uint32_t timeout_rx;        /**< receive timeout */
    uint32_t interval;          /**< connection event interval */
    uint16_t slave_latency; /**< count number of connection events, reset on new connection */

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

    /* pkt queues for passing data to and receiving data from l2cap layer */
    gorm_bufq_t rxq;
    gorm_bufq_t txq;
    gorm_buf_t *in_tx;
    gorm_buf_t *in_rx;
} gorm_ll_ctx_t;


void gorm_ll_host_init(void);


int gorm_ll_controller_init(netdev_t *dev);

int gorm_ll_adv_conn(gorm_ll_ctx_t *ctx, const gorm_gap_adv_ctx_t *adv_ctx);
int gorm_ll_adv_nonconn(gorm_ll_ctx_t *ctx, const gorm_gap_adv_ctx_t *adv_ctx);
int gorm_ll_adv_stop(gorm_ll_ctx_t *ctx);

int gorm_ll_terminate(gorm_ll_ctx_t *ctx);


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
// void gorm_ll_periph_init(void);
// void gorm_ll_periph_adv_start(void);
// void gorm_ll_periph_adv_setup(void *ad_adv, size_t adv_len,
//                               void *ad_scan, size_t scan_len);

/**
 * @brief   Close the given connection, independent of its state
 *
 * If the connection is currently in advertising or connected state, it will
 * be put into STANDBY state. If the connection is already STANDBY, nothing
 * will happen.
 *
 * @param[in,out] con   connection to close
 */
// void gorm_ll_periph_terminate(gorm_ll_ctx_t *con);
#endif

/**
 * @brief   Start advertising the given context
 *
 * @param con [description]
 * @return [description]
 */
// int gorm_ll_adv_start(gorm_ll_ctx_t *con);

#ifdef __cplusplus
}
#endif

#endif /* GORM_LL_H */
/** @} */
