/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_ll Link layer for Gorm
 * @ingroup     net_gorm
 * @brief       Gorm's link layer implementation
 * @{
 *
 * @file
 * @brief       Gorm's BLE link layer
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
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
/** @} */

/**
 * @brief   Link layer context for BLE connections
 */
typedef struct {
    /* connection state: holds ll state, flow control, and anchor state */
    uint8_t state;          /**< high-level state of this context */
    uint8_t flags;          /**< internal flags for tracking sub-states */

    /* context data when in connection state */
    uint16_t event_counter; /**< event counter for counting connection events */
    netdev_ble_ctx_t ctx;   /**< radio context */

    /* pointer to advertising configuration and data */
    // const gorm_gap_adv_ctx_t *gap;  /**< pointer to active advertising context */
    gorm_buf_t *sd_pkt;
    uint8_t adv_type;
    uint32_t adv_interval;

    /* remember address of connected peer */
    uint8_t peer_addr[BLE_ADDR_LEN];    /**< peer address when connected */

    /* channel configuration */
    uint8_t chan_hop;       /**< hop increment value */
    uint8_t chan_map[5];    /**< active channel map */
    uint8_t chan_cnt;       /**< number of active channels */
    uint8_t chan_unmapped;  /**< next unmapped channel */
    /* TODO: add channel selection algorithm as function pointer */

    /* used timers */
    gorm_arch_timer_t timer_spv;    /**< supervision timer */
    gorm_arch_timer_t timer_con;    /**< connection timer */

    /* timing parameters */
    uint32_t timeout_spv;   /**< supervision timeout */
    uint32_t timeout_rx;    /**< receive timeout */
    uint32_t interval;      /**< connection event interval */
    uint16_t slave_latency; /**< number of conn event we are allowed to miss */

    /* buffered timing parameter update values */
    struct {
        uint16_t instant;   /**< connection event when to apply these timings */
        uint8_t raw[9];     /**< new timing parameters */
    } timings_update;       /**< set of new connection parameters */

    /* buffered channel map update values */
    struct {
        uint16_t instant;   /**< connection event when to apply the new map */
        uint8_t map[5];     /**< new channel map to use */
        uint8_t cnt;        /**< number of active channels in the new map */
    } chan_update;          /**< set of new channel configuration parameters */

    /* pkt queues for passing data to and receiving data from l2cap layer */
    gorm_bufq_t rxq;        /**< packet queue for outgoing packets */
    gorm_bufq_t txq;        /**< packet queue for incoming packets */
    gorm_buf_t *in_tx;      /**< next packet to transfer */
    gorm_buf_t *in_rx;      /**< write next receive packet to this buffer */

#ifdef MODULE_GORM_STATS
    /* optional statistics for the last/currently active connection */
    struct {
        unsigned rx_cnt;    /**< number of packets received since connected */
        unsigned tx_cnt;    /**< number of packets sent since connected */
    } stats;                /**< collection of link layer statistics */
#endif
} gorm_ll_ctx_t;

/**
 * @brief   Initialize the lower part of the link layer
 *
 * @param[in] dev       device descriptor of the BLE radio
 *
 * @return  GORM_OK on success
 * @return  GORM_ERR_DEV if unable to initialize radio
 */
int gorm_ll_init(netdev_t *dev);

/**
 * @brief   Advertise given context
 *
 * @param[in,out] ctx   connection context, needs to be unused (STATE_STANDBY)
 * @param[in] adv_ctx   pre-configured advertising data
 * @param[in] mode      type of advertising, NON, UND, or DIR
 *
 * @return  0 on success
 * @return  TODO if no packet buffer for advertising data is available
 * @return  TODO if given context is busy
 */
int gorm_ll_adv(gorm_ll_ctx_t *ctx, const gorm_gap_adv_ctx_t *adv_ctx,
                gorm_gap_adv_mode_t mode);

/**
 * @brief   Cancel any operation the given context is involved in
 *
 * If the given context is connected, this connection will be closed. If the
 * context is being advertised, the advertising is stopped.
 *
 * @param[in,out] ctx   context to terminate
 */
void gorm_ll_terminate(gorm_ll_ctx_t *ctx);


static inline int gorm_ll_is_standby(gorm_ll_ctx_t *con)
{
    return con->state == GORM_LL_STATE_STANDBY;
}

#ifdef __cplusplus
}
#endif

#endif /* GORM_LL_H */
/** @} */
