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
 * @brief       Gorm's link layer implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <string.h>

#include "net/gorm/ll.h"
#include "net/gorm/ll/trx.h"
#include "net/gorm/ll/chan.h"
#include "net/gorm/host.h"
#include "net/gorm/util.h"
#include "net/gorm/config.h"
#include "net/gorm/arch/rand.h"

#define ENABLE_DEBUG                (1)
#include "debug.h"

#define TRANSMIT_WIN_DELAY          (1250UL)
#define TRANSMIT_WIN_DELAY_AUX      (2500UL)
#define TRANSMIT_WIN_DELAY_CODED    (3750UL)
#define T_MUL                       (1250UL)

/* boundaries for timing values */
#define INTERVAL_MIN                (7500UL)
#define INTERVAL_MAX                (4000000UL)
#define SUPER_TO_MIN                (100000UL)
#define SUPER_TO_MAX                (32000000UL)
#define WIN_SIZE_MAX                (10000UL)

#define HOP_MASK(x)                 (x & 0x1f)
#define SN(x)                       ((x >> 3) & 0x01)
#define NESN(x)                     ((x >> 2) & 0x01)

/* flags and fields used in the connection's state field */
#define FLAG_ANCHORED               (0x01)
#define FLAG_MOREDATA               (0x02)
#define FLAG_NESN                   (BLE_LL_NESN)       /* 0x04 */
#define FLAG_SN                     (BLE_LL_SN)         /* 0x08 */

#define TIMINGS_INVALID             (UINT32_MAX)

typedef struct {
    uint8_t win_size;
    uint8_t win_offset[2];
    uint8_t interval[2];
    uint8_t latency[2];
    uint8_t timeout[2];
} con_timings_raw_t;

typedef struct __attribute__((packed)) {
    uint8_t inita[BLE_ADDR_LEN];
    uint8_t adva[BLE_ADDR_LEN];
    uint8_t aa[4];
    uint8_t crc[BLE_CRC_LEN];
    con_timings_raw_t timings;
    uint8_t chan_map[5];
    uint8_t hop_sca;
} connect_ind_mtu_t;

static int _on_data_sent(gorm_buf_t *buf, void *arg);
static int _on_data_received(gorm_buf_t *buf, void *arg);
static void _on_connection_event_close(void *arg);

/* TODO: factor out to gorm_stats module */
static struct {
    unsigned rx_cnt;
} stats;

static const uint8_t adv_chan_list[] = GORM_CFG_LL_PERIPH_ADV_CHANNELS;

static void _build_ad_pkt(netdev_ble_pkt_t *pkt, uint8_t type,
                          const uint8_t *addr, const uint8_t *data, size_t len)
{
    pkt->flags = (type | BLE_LL_TXADD);
    pkt->len = (len + BLE_ADDR_LEN);
    memcpy(pkt->pdu, addr, BLE_ADDR_LEN);
    if (len > 0) {
        memcpy((pkt->pdu + BLE_ADDR_LEN), data, len);
    }
}

static uint32_t _update_timings(gorm_ll_ctx_t *con, uint32_t win_delay,
                                con_timings_raw_t *raw)
{
    /* parse and compute the needed values */
    con->timeout_spv = (uint32_t)(T_MUL * gorm_util_letohs(raw->timeout));
    con->timeout_rx = (uint32_t)(T_MUL * raw->win_size);
    con->interval = (uint32_t)(T_MUL * gorm_util_letohs(raw->interval));
    uint32_t offset = (uint32_t)(T_MUL * gorm_util_letohs(raw->win_offset));
    con->slave_latency = gorm_util_letohs(raw->latency);

    /* the anchor point of the connection event is the start of receival of the
     * incoming packet, so we need to compensate for the time passed */
    offset += win_delay;
    offset -= (GORM_CFG_LL_ANCHOR_OFFSET + GORM_CFG_LL_WIN_WIDENING);
    /* also we apply some static value for window widening */
    con->timeout_rx += (2 * GORM_CFG_LL_WIN_WIDENING);
    /* mark given raw timings as used */
    raw->win_size = 0;

    /* verify the new values */
    uint32_t win_size_max = ((con->interval - T_MUL) < WIN_SIZE_MAX) ?
                                (con->interval - T_MUL) : WIN_SIZE_MAX;
    if ((con->timeout_spv < SUPER_TO_MIN) ||
        (con->timeout_spv > SUPER_TO_MAX) ||
        (con->interval < INTERVAL_MIN) ||
        (con->interval > INTERVAL_MAX) ||
        (con->timeout_rx < T_MUL) ||
        (con->timeout_rx > win_size_max) ||
        (offset > con->interval)) {
        DEBUG("[gorm_ll] _update_timings: timings invalid\n");
        return TIMINGS_INVALID;
    }

    return offset - GORM_CFG_LL_RX_RAMPUP_DELAY;
}

static void _on_supervision_timeout(void *arg)
{
    gorm_ll_ctx_t *con = (gorm_ll_ctx_t *)arg;

    gorm_ll_trx_stop();
    gorm_arch_timer_cancel(&con->timer_con);
    con->state = GORM_LL_STATE_STANDBY;

    /* notify the host about the connection timeout */
    gorm_host_notify((gorm_ctx_t *)con, GORM_HOST_EVT_CON_TIMEOUT);

    DEBUG("[gorm_ll] on_supervision_timeout: CONNECTION CLOSED\n");
    DEBUG("stats:   rx counter:     %u\n", stats.rx_cnt);
    DEBUG("         event_counter:  %u\n", (unsigned)con->event_counter);
    DEBUG("         --- timings ---:\n");
    DEBUG("         timeout_spv:    %u\n", (unsigned)con->timeout_spv);
    DEBUG("         timeout_rx:     %u\n", (unsigned)con->timeout_rx);
    DEBUG("         interval:       %u\n", (unsigned)con->interval);
    DEBUG("         slave_latency:  %u\n", (unsigned)con->slave_latency);
}

static int _on_data_sent(gorm_buf_t *buf, void *arg)
{
    (void)buf;
    gorm_ll_ctx_t *con = (gorm_ll_ctx_t *)arg;

    /* try to allocate a new receive buffer if needed */
    if (!con->in_rx) {
        con->in_rx = gorm_buf_get();
    }

    /* if more data is expected, go on with this connection event */
    if (con->in_rx && (con->flags & FLAG_MOREDATA)) {
        con->flags &= ~(FLAG_MOREDATA);
        gorm_ll_trx_recv_next(con->in_rx, _on_data_received);
        gorm_arch_timer_set_from_now(&con->timer_con, con->timeout_rx,
                                     _on_connection_event_close, con);
    }
    else {
        _on_connection_event_close(con);
    }

    return 0;
}

static int _on_data_received(gorm_buf_t *buf, void *arg)
{
    gorm_ll_ctx_t *con = (gorm_ll_ctx_t *)arg;

    /* cancel receive timeout */
    gorm_arch_timer_cancel(&con->timer_con);

    /* check if this is the first packet of of the connection and anchor the
     * connection's timings if so */
    if (!(con->flags & FLAG_ANCHORED)) {
        gorm_arch_timer_anchor(&con->timer_con, GORM_CFG_LL_ANCHOR_OFFSET);
        con->flags |= FLAG_ANCHORED;
        con->timeout_rx = GORM_CFG_LL_RX_TIMEOUT;
    }

#if ENABLE_DEBUG
    stats.rx_cnt++;
#endif

    /* remember the flags field of the incoming packet */
    uint8_t flags = buf->pkt.flags;

    /* only handle incoming data, if the SN of the received packets is the same
     * as the sequence number that we expect. If SN does not fit, we silently
     * drop the incoming payload... */
    if (SN(flags) == NESN(con->flags) && (con->ctx.crc & NETDEV_BLE_CRC_OK)) {
        /* if the packet contains actual payload we pass it to the host */
        if (con->in_rx->pkt.len > 0) {
            gorm_buf_enq(&con->rxq, con->in_rx);
            gorm_host_notify((gorm_ctx_t *)con, GORM_HOST_EVT_DATA);
            con->in_rx = NULL;
        }
        /* and we ACK the incoming packet */
        con->flags ^= BLE_LL_NESN;
    }

    /* now building the response:
     * - if last packet was not ACKed, we resend it
     * - else we either reset the current packet, or get the next packet from
     *   the transmit queue */
    if (SN(con->flags) != NESN(flags)) {    /* last packet was sent ok */
        /* increment sequence number */
        con->flags ^= BLE_LL_SN;
        /* if more data is waiting to be send we get the next buffer */
        if (gorm_buf_peek(&con->txq)) {
            gorm_buf_return(con->in_tx);
            con->in_tx = gorm_buf_deq(&con->txq);
        }
        /* else we stick with the current buffer */
        else {
            con->in_tx->pkt.flags = BLE_LL_LLID_DATA_CONT;
            con->in_tx->pkt.len = 0;
        }
    }

    /* finish setting the reply packet's flags */
    con->in_tx->pkt.flags &= ~(BLE_LL_FLOW_MASK);
    con->in_tx->pkt.flags |= (con->flags & BLE_LL_SEQ_MASK);
    if (gorm_buf_peek(&con->txq)) {
        con->in_tx->pkt.flags |= BLE_LL_MD;
    }

    /* remember if there will be potentially more packets in this event */
    if ((con->in_tx->pkt.flags & BLE_LL_MD) || (flags & BLE_LL_MD)) {
        con->flags |= FLAG_MOREDATA;
    }

    /* send data */
    gorm_ll_trx_send_next(con->in_tx, _on_data_sent);

    /* and refresh supervision timer */
    gorm_arch_timer_set_from_now(&con->timer_spv, con->timeout_spv,
                                 _on_supervision_timeout, con);

    // LED4_ON;

    return 0;
}

static void _on_connection_event_start(void *arg)
{
    gorm_ll_ctx_t *con = (gorm_ll_ctx_t *)arg;
    int noskip = 1;

    // LED5_OFF;

    /* increase the event counter */
    con->event_counter++;

    /* TODO: check if radio is busy, and if so skip this event */
    /* TODO: add skipping of connection events up to (slave_latency - CFG),
     *       but only if there is no new data to be send */
    /* TODO: reverse if condition, use || and test for 'good' case... */
    // if ((!gorm_buf_peek(&con->txq)) && (!con->in_tx) &&
    //     (TODO con->skipped + CFG_SKIPP_GUARD < con->slave_latency) &&
    //     (!con->flags & FLAG_ANCHORED)) {
    //     ++con->skipped;
    //     gorm_arch_timer_set_from_last(&con->connection_timer,
    //                                   (con->win_size + con->win_spacing));
    // }

    /* make sure we have a buffer allocated for receiving the next packet */
    if (!con->in_rx) {
        con->in_rx = gorm_buf_get();
    }
    /* put radio into RX mode (but only if a RX buffer is available) */
    if (noskip && con->in_rx) {
        /* de-anchor event slot */
        con->flags &= ~(FLAG_ANCHORED);
        /* listen for incoming packets now */
        gorm_ll_trx_recv(con->in_rx, &con->ctx, _on_data_received, con);
        // /* and set timeout to close the transmit window again */
        gorm_arch_timer_set_from_now(&con->timer_con, con->timeout_rx,
                                     _on_connection_event_close, con);
    } else {
        /* skip this event */
        _on_connection_event_close(con);
    }
}

static void _on_connection_event_close(void *arg)
{
    gorm_ll_ctx_t *con = (gorm_ll_ctx_t *)arg;


    /* cancel receive operation */
    gorm_ll_trx_stop();


    /* check if we need to update the channel map */
    if ((con->chan_update.cnt != 0) &&
        (con->chan_update.instant == con->event_counter)) {
        memcpy(con->chan_map, con->chan_update.map, BLE_CHANMAP_LEN);
        con->chan_cnt = con->chan_update.cnt;
        con->chan_update.cnt = 0;
    }
    /* calculate channel to be used in next connection event */
    /* TODO: enable to use algo #2 also... */
    // con->chan_algo(con);
    gorm_ll_chan_algo1(con);

    /* offset to the next window start */
    uint32_t offset = con->interval;
    /* check if we have a pending connection update waiting */
    if ((con->timings_update.raw[0] != 0) &&
        (con->timings_update.instant == con->event_counter + 1)) {
        con->flags &= ~FLAG_ANCHORED;
        offset += _update_timings(con, 0,
                                  (con_timings_raw_t *)con->timings_update.raw);
    }

    /* setup the next connection event */
    gorm_arch_timer_set_from_last(&con->timer_con, offset,
                                  _on_connection_event_start, con);
}

static void _connect(gorm_ll_ctx_t *con, gorm_buf_t *buf)
{
    /* first thing: remember this point in time and anchor the connection timer
     * to it */
    gorm_arch_timer_cancel(&con->timer_con);
    gorm_arch_timer_cancel(&con->timer_spv);
    gorm_arch_timer_anchor(&con->timer_con, 0);

    /* we are now officially in CONNECTED state with a fresh sequence number
     * and start at event 0 (will be incremented at the start of each connection
     * event) */
    con->state = GORM_LL_STATE_CONN;
    con->flags = BLE_LL_SN;
    con->event_counter = UINT16_MAX;

    /* parse the CONNECT_IND payload */
    connect_ind_mtu_t *lldata = (connect_ind_mtu_t *)buf->pkt.pdu;

    /* remember the address of the connected peer */
    memcpy(con->peer_addr, lldata->inita, BLE_ADDR_LEN);

    /* prepare the basic radio context. The values in the PDU are not
     * necessarily word aligned, so use memcpy to copy them */
    memcpy(con->ctx.aa.raw, lldata->aa, sizeof(uint32_t));
    memcpy(&con->ctx.crc, lldata->crc, BLE_CRC_LEN);

    /* prepare the channel selection context. So far we only support channel
     * selection algorithm #1... */
    /* TODO: support algorithm #2 */
    // con->chan_algo = (con->in_rx->flags & BLE_LL_CHSEL) ?
    //                  gorm_ll_chan_algo2 : gorm_ll_chan_algo1;
    if (con->in_rx->pkt.flags & BLE_LL_CHSEL) {
        DEBUG("[gorm_ll] _connect: channel sel algo #2 not supported\n");
        goto err;
    }
    memcpy(con->chan_map, lldata->chan_map, BLE_CHANMAP_LEN);
    con->chan_cnt = gorm_ll_chan_count(con->chan_map);
    con->chan_hop = HOP_MASK(lldata->hop_sca);
    if ((con->chan_hop < 5) || (con->chan_hop > 16)) {
        DEBUG("[gorm_ll] _connect: channel hop value invalid\n");
        goto err;
    }
    con->chan_unmapped = 0;
    /* calculate the channel we use for the first connection event */
    gorm_ll_chan_algo1(con);

    /* parse timings and set timer */
    uint32_t offset = _update_timings(con, TRANSMIT_WIN_DELAY, &lldata->timings);
    if (offset == TIMINGS_INVALID) {
        DEBUG("[gorm_ll] _connect: invalid timings\n");
        goto err;
    }

    /* let the magic begin! */
    gorm_arch_timer_set_from_now(&con->timer_spv, con->timeout_spv,
                                 _on_supervision_timeout, con);
    gorm_arch_timer_set_from_last(&con->timer_con, offset,
                                  _on_connection_event_start, con);
    /* and notify the host about the new connection */
    gorm_host_notify((gorm_ctx_t *)con, GORM_HOST_EVT_CONNECTED);
    /* now we are in connected state */
    return;

err:
    con->state = GORM_LL_STATE_STANDBY;
    /* TODO: notify host. Is there actually anything to tell the host?
     *       No state change if the connection fails to be established...
     *       BUT we should continue to advertise. */
    gorm_host_notify((gorm_ctx_t *)con, GORM_HOST_EVT_CON_ABORT);
}

static int _on_adv_reply(gorm_buf_t *buf, void *arg)
{
    gorm_ll_ctx_t *con = (gorm_ll_ctx_t *)arg;

    /* drop packet if the CRC is not ok */
    if (!(con->ctx.crc & NETDEV_BLE_CRC_OK)) {
        return 1;
    }

    /* TODO: verify target address against our own address */
    /* TODO: do some more general verifications:
     *       - is connection in correct (ADV) state?
     *       - more? */

    /* and do something about it, if we know how */
    switch (buf->pkt.flags & BLE_LL_PDU_MASK) {
        case BLE_LL_SCAN_REQ: {
            _build_ad_pkt(&con->in_tx->pkt, BLE_LL_SCAN_RESP, con->gap->addr,
                          con->gap->scan_data, con->gap->scan_len);
            gorm_ll_trx_send_next(con->in_tx, NULL);
            return 0;
        }
        case BLE_LL_CONNECT_IND: {
            _connect(con, buf);
            break;
        }
    }

    /* there will be no other transmission in this slot */
    return 1;
}

static int _on_adv_sent(gorm_buf_t *buf, void *arg)
{
    (void)buf;
    gorm_ll_ctx_t *con = (gorm_ll_ctx_t *)arg;
    gorm_ll_trx_recv_next(con->in_rx, _on_adv_reply);
    return 0;
}

static void _on_adv_chan(void *arg)
{
    gorm_ll_ctx_t *con = (gorm_ll_ctx_t *)arg;

    /* stop listening for connection requests and the like */
    /* TODO: make sure we only stop our own operation here... */
    /* TODO: we skip the adv event if someone else is using the radio right now */
    gorm_ll_trx_stop();

    if (con->event_counter < sizeof(adv_chan_list)) {
        /* build packet */
        _build_ad_pkt(&con->in_tx->pkt, BLE_LL_ADV_IND, con->gap->addr,
                      con->gap->adv_data, con->gap->adv_len);
        con->ctx.aa.u32 = GORM_LL_ADV_AA;
        con->ctx.crc = GORM_LL_ADV_CRC;
        con->ctx.chan = adv_chan_list[con->event_counter];
        /* send out advertisement packet */
        gorm_ll_trx_send(con->in_tx, &con->ctx, _on_adv_sent, con);
        /* set timeout / start of next advertising channel */
        /* TODO: still set this in case we skip the event */
        gorm_arch_timer_set_from_now(&con->timer_con,
                                     GORM_CFG_LL_PERIPH_ADV_EVENT_DURATION,
                                     _on_adv_chan, con);
        con->event_counter++;
    }
    else {
        con->event_counter = 0;
    }
}

static void _on_adv_event(void *arg)
{
    gorm_ll_ctx_t *con = (gorm_ll_ctx_t *)arg;

    /* schedule next advertising event. Add a [0,10ms] `advDelay` to the
     * `advInterval`
     * -> spec v5.0-vol6-b-4.4.2.2.1 */
    uint32_t next = (con->gap->interval + gorm_arch_rand(0, 10000));
    gorm_arch_timer_set_from_last(&con->timer_spv, next, _on_adv_event, con);

    /* trigger the actual sending and receiving of advertisement data */
    _on_adv_chan(con);
}

static int _advertise(gorm_ll_ctx_t *con, const gorm_gap_adv_ctx_t *adv_ctx)
{
    con->in_tx = gorm_buf_get();
    if (!con->in_tx) {
        DEBUG("[gorm_ll] _advertise: unable to allocate tx buffer\n");
        con->state = GORM_LL_STATE_STANDBY;
        if (con->in_tx) {
            gorm_buf_return(con->in_rx);
            con->in_tx = NULL;
        }
        return GORM_ERR_NOBUF;
    }

    /* reset general state and start advertising */
    con->event_counter = 0;
    con->gap = adv_ctx;
    gorm_arch_timer_anchor(&con->timer_spv, 0);
    gorm_arch_timer_set_from_last(&con->timer_spv, con->gap->interval,
                                  _on_adv_event, con);

    DEBUG("[gorm_ll] _advertise: starting to advertise now\n");
    return GORM_OK;
}

int gorm_ll_controller_init(netdev_t *dev)
{
    DEBUG("[gorm_ll] controller_init: initializing controller\n");
    if (gorm_ll_trx_init(dev) != 0) {
        return GORM_ERR_DEV;
    }

    return GORM_OK;
}

int gorm_ll_adv_conn(gorm_ll_ctx_t *ctx, const gorm_gap_adv_ctx_t *adv_ctx)
{
    assert(ctx && adv_ctx);

    if (ctx->state != GORM_LL_STATE_STANDBY) {
        return GORM_ERR_CTX_BUSY;
    }

    /* allocate RX buffer, needed as we want to listen for scan requests and
     * connection indications */
    ctx->in_rx = gorm_buf_get();
    if (!ctx->in_rx) {
        DEBUG("[gorm_ll] adv_conn: unable to allocate rx buffer\n");
        return GORM_ERR_NOBUF;
    }

    ctx->state = GORM_LL_STATE_ADV;
    return _advertise(ctx, adv_ctx);
}

int gorm_ll_adv_nonconn(gorm_ll_ctx_t *ctx, const gorm_gap_adv_ctx_t *adv_ctx)
{
    assert(ctx && adv_ctx);

    if (ctx->state != GORM_LL_STATE_STANDBY) {
        return GORM_ERR_CTX_BUSY;
    }

    ctx->state = GORM_LL_STATE_ADV_ONLY;
    return _advertise(ctx, adv_ctx);
}

int gorm_ll_adv_stop(gorm_ll_ctx_t *ctx)
{
    assert(ctx);

    /* TODO */
    return GORM_OK;
}

int gorm_ll_terminate(gorm_ll_ctx_t *con)
{
    assert(con);

    // unsigned is = irq_disable();
    // gorm_ll_trx_stop();
    // gorm_arch_timer_cancel(&con->timer_con);
    // gorm_arch_timer_cancel(&con->timer_spv);
    // con->state = STATE_STANDBY;
    // irq_restore(is);

    /* TODO */
    return GORM_OK;
}
