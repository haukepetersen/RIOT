/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
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
 * @brief       Gorm's link layer implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <string.h>
#include <limits.h>

#include "net/ble.h"
#include "net/gorm/ll.h"
#include "net/gorm/ll/chan.h"
#include "net/gorm/ll/trx.h"
#include "net/gorm/ll/host.h"
#include "net/gorm/gap.h"
#include "net/gorm/util.h"
#include "net/gorm/pdupool.h"
#include "net/gorm/arch/rand.h"
#include "net/gorm/arch/timer.h"

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
#define FLAG_NESN                   (GORM_LL_NESN)      /* 0x04 */
#define FLAG_SN                     (GORM_LL_SN)        /* 0x08 */
#define STATE_MASK                  (0xf0)
#define STATE_ADV                   (0x10)
#define STATE_INIT                  (0x20)
#define STATE_CONN                  (0x30)
#define STATE_STANDBY               (0x40)
#define STATE_SCAN                  (0x50)

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


static struct {
    unsigned rx_cnt;
} stats;

static const uint8_t adv_chan_list[] = GORM_CFG_LL_PERIPH_ADV_CHANNELS;

static netdev_ble_ctx_t adv_ctx = {
    .aa.u32 = GORM_LL_ADV_AA,
    .crc = GORM_LL_ADV_CRC,
    .chan = 0
};

/* TODO: move back into connection data? Or just move up-stack?! */
struct {
    /* TODO: cater for different addresses (public, randon, private) */
    /* TODO: how to save TxAdd flag? for marking public addresses? */
    uint8_t addr[BLE_ADDR_LEN];
    uint8_t *ad_adv;
    size_t ad_adv_len;
    uint8_t *ad_scan;
    size_t ad_scan_len;
    /* TODO: move this and a pointer to this this struct into con_t */
    uint8_t adv_chan;
} adv_data;

/* TODO: move allocation and handling of connections up the stack */
static gorm_ll_connection_t connections[GORM_CFG_LL_PERIPH_CONNECTIONS_LIMIT];

static uint8_t _get_state(gorm_ll_connection_t *con)
{
    return (con->state & STATE_MASK);
}

/* TODO: remove this function once we move the connection memory handling up the
 *       stack */
static gorm_ll_connection_t *_get_by_state(uint8_t state)
{
    for (unsigned i = 0; i < GORM_CFG_LL_PERIPH_CONNECTIONS_LIMIT; i++) {
        if (_get_state(&connections[i]) == state) {
            return &connections[i];
        }
    }
    return NULL;
}

static void _build_ad_pkt(netdev_ble_pkt_t *pkt,
                          uint8_t type, uint8_t *ad_data, size_t ad_data_len)
{
    assert((ad_data_len + BLE_ADDR_LEN) <= NETDEV_BLE_PDU_MAXLEN);

    /* TODO: public address handling needs to be generalized... */
    pkt->flags = (type | GORM_LL_FLAG_TXADD);
    pkt->len = (ad_data_len + BLE_ADDR_LEN);
    memcpy(pkt->pdu, adv_data.addr, BLE_ADDR_LEN);
    if (ad_data) {
        memcpy((pkt->pdu + BLE_ADDR_LEN), ad_data, ad_data_len);
    }
}

static uint32_t _update_timings(gorm_ll_connection_t *con, uint32_t win_delay,
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

    /* Verification of values */
    uint32_t win_size_max = ((con->interval - T_MUL) < WIN_SIZE_MAX) ?
                                (con->interval - T_MUL) : WIN_SIZE_MAX;
    if ((con->timeout_spv < SUPER_TO_MIN) ||
        (con->timeout_spv > SUPER_TO_MAX) ||
        (con->interval < INTERVAL_MIN) ||
        (con->interval > INTERVAL_MAX) ||
        (con->timeout_rx < T_MUL) ||
        (con->timeout_rx > win_size_max) ||
        (offset > con->interval)) {
        DEBUG("[gorm_ll_periph] _update_timings: timings invalid\n");
        return TIMINGS_INVALID;
    }

    return offset - GORM_CFG_LL_RX_RAMPUP_DELAY;
}

static void _on_supervision_timeout(void *arg)
{
    gorm_ll_connection_t *con = (gorm_ll_connection_t *)arg;

    gorm_ll_trx_stop();
    gorm_arch_timer_cancel(&con->timer_con);
    con->state = STATE_STANDBY;

    /* TODO: notify host about lost connection */

    DEBUG("[gorm_ll_periph] on_supervision_timeout: CONNECTION CLOSED\n");
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
    gorm_ll_connection_t *con = (gorm_ll_connection_t *)arg;

    /* try to allocate a new receive buffer if needed */
    if (!con->in_rx) {
        con->in_rx = gorm_pdupool_get(1);
    }

    /* if more data is expected, go on with this connection event */
    if (con->in_rx && (con->state & FLAG_MOREDATA)) {
        con->state &= ~(FLAG_MOREDATA);
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
    gorm_ll_connection_t *con = (gorm_ll_connection_t *)arg;

    // LED4_OFF;

    /* cancel receive timeout */
    gorm_arch_timer_cancel(&con->timer_con);

    /* check if this is the first packet of of the connection and anchor the
     * connection's timings if so */
    if (!(con->state & FLAG_ANCHORED)) {
        gorm_arch_timer_anchor(&con->timer_con, GORM_CFG_LL_ANCHOR_OFFSET);
        con->state |= FLAG_ANCHORED;
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
    if (SN(flags) == NESN(con->state) && (con->ctx.crc & NETDEV_BLE_CRC_OK)) {
        /* if the packet contains actual payload we pass it to the host */
        if (con->in_rx->pkt.len > 0) {
            gorm_pduq_enq(&con->rxq, con->in_rx);
            gorm_ll_host_notify(con);
            con->in_rx = NULL;
        }
        /* and we ACK the incoming packet */
        con->state ^= GORM_LL_NESN;
    }

    /* now building the response:
     * - if last packet was not ACKed, we resend it
     * - else we either reset the current packet, or get the next packet from
     *   the transmit queue */
    if (SN(con->state) != NESN(flags)) {    /* last packet was sent ok */
        /* increment sequence number */
        con->state ^= GORM_LL_SN;
        /* if more data is waiting to be send we get the next buffer */
        if (gorm_pduq_peek(&con->txq)) {
            gorm_pdupool_return(con->in_tx);
            con->in_tx = gorm_pduq_deq(&con->txq);
        }
        /* else we stick with the current buffer */
        else {
            con->in_tx->pkt.flags = GORM_LL_LLID_DATA_CONT;
            con->in_tx->pkt.len = 0;
        }
    }

    /* finish setting the reply packet's flags */
    con->in_tx->pkt.flags &= ~(GORM_LL_FLOW_MASK);
    con->in_tx->pkt.flags |= (con->state & GORM_LL_SEQ_MASK);
    if (gorm_pduq_peek(&con->txq)) {
        con->in_tx->pkt.flags |= GORM_LL_MD;
    }

    /* remember if there will be potentially more packets in this event */
    if ((con->in_tx->pkt.flags & GORM_LL_MD) || (flags & GORM_LL_MD)) {
        con->state |= FLAG_MOREDATA;
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
    gorm_ll_connection_t *con = (gorm_ll_connection_t *)arg;
    int noskip = 1;

    // LED5_OFF;

    /* increase the event counter */
    con->event_counter++;

    /* TODO: check if radio is busy, and if so skip this event */
    /* TODO: add skipping of connection events up to (slave_latency - CFG),
     *       but only if there is no new data to be send */
    /* TODO: reverse if condition, use || and test for 'good' case... */
    // if ((!gorm_pduq_peek(&con->txq)) && (!con->in_tx) &&
    //     (TODO con->skipped + CFG_SKIPP_GUARD < con->slave_latency) &&
    //     (!con->state & FLAG_ANCHORED)) {
    //     ++con->skipped;
    //     gorm_arch_timer_set_from_last(&con->connection_timer,
    //                                   (con->win_size + con->win_spacing));
    // }

    /* make sure we have a buffer allocated for receiving the next packet */
    if (!con->in_rx) {
        con->in_rx = gorm_pdupool_get(1);
    }
    /* put radio into RX mode (but only if a RX buffer is available) */
    if (noskip && con->in_rx) {
        /* de-anchor event slot */
        con->state &= ~(FLAG_ANCHORED);
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
    gorm_ll_connection_t *con = (gorm_ll_connection_t *)arg;


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
        con->state &= ~FLAG_ANCHORED;
        offset += _update_timings(con, 0,
                                  (con_timings_raw_t *)con->timings_update.raw);
    }

    /* setup the next connection event */
    gorm_arch_timer_set_from_last(&con->timer_con, offset,
                                  _on_connection_event_start, con);

    // LED5_ON;
}

static void _connect(gorm_ll_connection_t *con, gorm_buf_t *buf)
{
    /* first thing: remember this point in time and anchor the connection timer
     * to it */
    // LED4_OFF;
    gorm_arch_timer_cancel(&con->timer_con);
    gorm_arch_timer_cancel(&con->timer_spv);
    gorm_arch_timer_anchor(&con->timer_con, 0);

    /* we are now officially in CONNECTED state with a fresh sequence number
     * and start at event 0 (will be incremented at the start of each connection
     * event) */
    con->state = (STATE_CONN | GORM_LL_SN);
    con->event_counter = UINT16_MAX;

    /* parse the CONNECT_IND payload */
    connect_ind_mtu_t *lldata = (connect_ind_mtu_t *)buf->pkt.pdu;

    /* TODO: save initiator address? byte 0-5... not sure we need this...*/

    /* prepare the basic radio context. The values in the PDU are not
     * necessarily word aligned, so use memcpy to copy them */
    memcpy(con->ctx.aa.raw, lldata->aa, sizeof(uint32_t));
    memcpy(&con->ctx.crc, lldata->crc, BLE_CRC_LEN);

    /* prepare the channel selection context. So far we only support channel
     * selection algorithm #1... */
    /* TODO: support algorithm #2 */
    // con->chan_algo = (con->in_rx->flags & GORM_LL_FLAG_CHSEL) ?
    //                  gorm_ll_chan_algo2 : gorm_ll_chan_algo1;
    if (con->in_rx->pkt.flags & GORM_LL_FLAG_CHSEL) {
        DEBUG("[gorm_ll_periph] _connect: channel sel algo #2 not supported\n");
        goto err;
    }
    memcpy(con->chan_map, lldata->chan_map, BLE_CHANMAP_LEN);
    con->chan_cnt = gorm_ll_chan_count(con->chan_map);
    con->chan_hop = HOP_MASK(lldata->hop_sca);
    if ((con->chan_hop < 5) || (con->chan_hop > 16)) {
        DEBUG("[gorm_ll_periph] _connect: channel hop value invalid\n");
        goto err;
    }
    con->chan_unmapped = 0;
    /* calculate the channel we use for the first connection event */
    gorm_ll_chan_algo1(con);

    /* parse timings and set timer */
    uint32_t offset = _update_timings(con, TRANSMIT_WIN_DELAY, &lldata->timings);
    if (offset == TIMINGS_INVALID) {
        DEBUG("[gorm_ll_periph] _connect: invalid timings\n");
        goto err;
    }

    /* let the magic begin! */
    gorm_arch_timer_set_from_now(&con->timer_spv, con->timeout_spv,
                                 _on_supervision_timeout, con);
    gorm_arch_timer_set_from_last(&con->timer_con, offset,
                                  _on_connection_event_start, con);
    /* TODO: notify host about new connection */
    /* TODO: continue advertising on the next free connection struct
     *       (if available) -> trigger this from host! */
    /* now we are in connected state */
    // LED4_ON;
    return;

err:
    con->state = STATE_STANDBY;
    /* TODO: notify host. Is there actually anything to tell the host?
     *       No state change if the connection fails to be established...
     *       BUT we should continue to advertise. */
}

static int _on_adv_reply(gorm_buf_t *buf, void *arg)
{
    /* NOTE: this is run in interrupt context! */
    gorm_ll_connection_t *con = (gorm_ll_connection_t *)arg;

    /* drop packet if the CRC is not ok */
    if (!(adv_ctx.crc & NETDEV_BLE_CRC_OK)) {
        return 1;
    }

    /* TODO: verify target address against our own address */
    /* TODO: do some more general verifications:
     *       - is connection in correct (ADV) state?
     *       - more? */

    /* and do something about it, if we know how */
    switch (gorm_ll_get_type(&buf->pkt)) {
        case GORM_LL_SCAN_REQ: {
            _build_ad_pkt(&con->in_tx->pkt, GORM_LL_SCAN_RESP,
                          adv_data.ad_scan, adv_data.ad_scan_len);
            gorm_ll_trx_send_next(con->in_tx, NULL);
            return 0;
        }
        case GORM_LL_CONNECT_IND: {
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
    gorm_ll_connection_t *con = (gorm_ll_connection_t *)arg;
    gorm_ll_trx_recv_next(con->in_rx, _on_adv_reply);
    return 0;
}

static void _on_adv_chan(void *arg)
{
    gorm_ll_connection_t *con = (gorm_ll_connection_t *)arg;

    /* stop listening for connection requests and the like */
    /* TODO: make sure we only stop our own operation here... */
    /* TODO: we skip the adv event if someone else is using the radio right now */
    gorm_ll_trx_stop();

    if (adv_data.adv_chan < sizeof(adv_chan_list)) {
        /* build packet */
        _build_ad_pkt(&con->in_tx->pkt, GORM_LL_ADV_IND,
                      adv_data.ad_adv, adv_data.ad_adv_len);
        adv_ctx.chan = adv_chan_list[adv_data.adv_chan];
        /* send out advertisement packet */
        gorm_ll_trx_send(con->in_tx, &adv_ctx, _on_adv_sent, con);
        /* set timeout / start of next advertising channel */
        /* TODO: still set this in case we skip the event */
        gorm_arch_timer_set_from_now(&con->timer_con,
                                     GORM_CFG_LL_PERIPH_ADV_EVENT_DURATION,
                                     _on_adv_chan, con);
        adv_data.adv_chan++;
    }
    else {
        adv_data.adv_chan = 0;
    }
}

static void _on_adv_event(void *arg)
{
    gorm_ll_connection_t *con = (gorm_ll_connection_t *)arg;

    /* schedule next advertising event. Add a [0,10ms] `advDelay` to the
     * `advInterval`
     * -> spec v5.0-vol6-b-4.4.2.2.1 */
    uint32_t next = (GORM_CFG_LL_PERIPH_ADV_INTERVAL + gorm_arch_rand(0, 10000));
    gorm_arch_timer_set_from_last(&con->timer_spv, next, _on_adv_event, con);

    /* trigger the actual sending and receiving of advertisement data */
    _on_adv_chan(con);
}

void gorm_ll_periph_init(void)
{
    /* address(es) */
    /* TODO: refine and fix... */
    memcpy(adv_data.addr, gorm_ll_addr_rand(), BLE_ADDR_LEN);

    /* initialize connection memory */
    /* TODO: move up the stack to host?! */
    for (unsigned i = 0; i < GORM_CFG_LL_PERIPH_CONNECTIONS_LIMIT; i++) {
        memset(&connections[i], 0, sizeof(gorm_ll_connection_t));
        connections[i].state = STATE_STANDBY;
    }
}

gorm_ll_connection_t *gorm_ll_periph_getcon(void)
{
    return connections;
}

void gorm_ll_periph_adv_setup(void *ad_adv, size_t adv_len,
                              void *ad_scan, size_t scan_len)
{
    adv_data.ad_adv = ad_adv;
    adv_data.ad_adv_len = adv_len;
    adv_data.ad_scan = ad_scan;
    adv_data.ad_scan_len = scan_len;
}

void gorm_ll_periph_adv_start(void)
{
    DEBUG("[gorm_ll_periph] adv_start: trying to trigger advertising now\n");

    /* make sure we are not advertising at the moment */
    /* TODO: solve this using some global state variable/pointer? */
    if (_get_by_state(STATE_ADV) != NULL) {
        DEBUG("[gorm_ll_periph] adv_start: advertisement in progress...\n");
        return;
    }

    /* get any non-connected slot */
    /* TODO: use conn_interval for this, get rid of state field */
    gorm_ll_connection_t *con = _get_by_state(STATE_STANDBY);
    if (con == NULL) {
        DEBUG("[gorm_ll_periph] adv_start: not possible: all slots taken\n");
        return;
    }

    /* allocate two empty PDUs as rx and tx buffers. We do this here, as this
     * this makes sure we can enter a connection state, and do not have to drop
     * the connection right away during _connect() in case no PDU is available
     * from the pool. */
    con->in_rx = gorm_pdupool_get(2);
    if (!con->in_rx) {
        DEBUG("[gorm_ll_periph] adv_start: unable to allocate rx/tx buffers\n");
        return;
    }
    con->in_tx = con->in_rx->next;

    /* setup the the timers to use for advertising interval and rx timeout */
    con->state = STATE_ADV;
    adv_data.adv_chan = 0;

    /* start advertising now */
    gorm_arch_timer_anchor(&con->timer_spv, 0);
    gorm_arch_timer_set_from_last(&con->timer_spv,
                                  GORM_CFG_LL_PERIPH_ADV_INTERVAL,
                                  _on_adv_event, con);

    /* cleanup */
    DEBUG("[gorm_ll_periph] adv: starting to advertise now\n");
}

void gorm_ll_periph_terminate(gorm_ll_connection_t *con)
{
    unsigned is = irq_disable();
    gorm_ll_trx_stop();
    gorm_arch_timer_cancel(&con->timer_con);
    gorm_arch_timer_cancel(&con->timer_spv);
    con->state = STATE_STANDBY;
    irq_restore(is);
}
