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
 * @brief       Bootstrapping Gorm
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <string.h>

#include "log.h"

#include "net/gorm.h"
#include "net/gorm/ll.h"
#include "net/gorm/ll/ctrl.h"
#include "net/gorm/l2cap.h"

#ifdef MODULE_GORM_GATT
#include "net/gorm/gatt.h"
#endif

#define ENABLE_DEBUG    (1)
#include "debug.h"

#define CTX_NUMOF                   GORM_CFG_CONNECTION_NUMOF

// #define STATE_STANDBY               (0U)
// #define STATE_ADVERTISING           (1U)

/* internal state, used to keep track of global advertisement status */
// static uint8_t _state = STATE_STANDBY;

/* allocate memory for the defined number of possible connections */
// static gorm_ctx_t _ctx[CTX_NUMOF];

static gorm_event_handler_t *_app_handler = NULL;

/* allocate initial buffer memory */
static gorm_buf_t _buf_pool[GORM_CFG_POOLSIZE];


// static gorm_ctx_t *_get_by_state(uint8_t state)

// {
//     for (unsigned i = 0; i < CTX_NUMOF; i++) {
//         if (_ctx[i].ll.state == state) {
//             return &_ctx[i];
//         }
//     }
//     return NULL;
// }

static void _on_data(gorm_ctx_t *con)
{
    gorm_buf_t *buf;
    while ((buf = gorm_buf_deq(&con->ll.rxq))) {
        uint8_t llid = (buf->pkt.flags & BLE_LL_LLID_MASK);

        switch (llid) {
            case BLE_LL_LLID_CTRL:
                gorm_ll_ctrl_on_data(con, buf);
                break;
            case BLE_LL_LLID_DATA_CONT:
            case BLE_LL_LLID_DATA_START:
                gorm_l2cap_on_data(con, llid, buf);
                break;
            default:
                DEBUG("[gorm] on_data: invalid LLID value\n");
                gorm_buf_return(buf);
                break;
        }
    }
}

static void _notify_app(gorm_ctx_t *ctx, uint16_t event)
{
    gorm_event_handler_t *handler = _app_handler;
    while (handler) {
        handler->event_cb(ctx, event);
        handler = handler->next;
    }
}

static void _on_event(gorm_arch_evt_t *event)
{
    gorm_ctx_t *ctx = event->ctx;

    /* read pending events */
    unsigned is = irq_disable();
    uint16_t state = event->state;
    event->state = 0;
    irq_restore(is);

    /* TODO: cleanup code */
    while (state) {
        /* NOTE: the order in which we check the events matters! */
        if (state & GORM_EVT_CONN_TIMEOUT) {
            DEBUG("[gorm] connection timeout\n");
            DEBUG("stats: event_counter:        %u\n", (unsigned)ctx->ll.event_counter);
#ifdef MODULE_GORM_STATS
            DEBUG("       packets transmitted:  %u\n", ctx->ll.stats.tx_cnt);
            DEBUG("       packets received:     %u\n", ctx->ll.stats.rx_cnt);
            DEBUG("       last chan map update: %u\n", ctx->ll.chan_update.instant);
            DEBUG("       last param update:    %u\n", ctx->ll.timings_update.instant);
#endif
            DEBUG("       --- timings ---:\n");
            DEBUG("       timeout_spv:          %u\n", (unsigned)ctx->ll.timeout_spv);
            DEBUG("       timeout_rx:           %u\n", (unsigned)ctx->ll.timeout_rx);
            DEBUG("       interval:             %u\n", (unsigned)ctx->ll.interval);
            DEBUG("       slave_latency:        %u\n", (unsigned)ctx->ll.slave_latency);

            _notify_app(ctx, GORM_EVT_CONN_TIMEOUT);
            state &= ~GORM_EVT_CONN_TIMEOUT;
        }
        if (state & GORM_EVT_CONN_CLOSED) {
            DEBUG("[gorm] connection closed\n");
            _notify_app(ctx, GORM_EVT_CONN_CLOSED);
            state = 0;
            // TODO: state cleanup -> clear ctx and all included COCs etc
        }
        if (state & GORM_EVT_CONN_ABORT) {
            DEBUG("[gorm] connection attempt aborted\n");
            _notify_app(ctx, GORM_EVT_CONN_ABORT);
            state = 0;
        }
        if (state & GORM_EVT_CONNECTED) {
            DEBUG("[gorm] connection established\n");
            _notify_app(ctx, GORM_EVT_CONNECTED);
            state &= ~(GORM_EVT_CONNECTED);
        }
        if (state & GORM_EVT_DATA) {
            _on_data(ctx);
            if (gorm_buf_peek(&ctx->ll.rxq) == NULL) {
                state &= ~(GORM_EVT_DATA);
            }
        }
    }
}

void gorm_init(netdev_t *dev)
{
    /* initialize buffer pool */
    gorm_buf_add_buffers(_buf_pool, GORM_CFG_POOLSIZE);
    DEBUG("[gorm] initialized buffer pool\n");

    /* initializing the link layer */
    if (gorm_ll_init(dev) != GORM_OK) {
        LOG_ERROR("[gorm] error: unable to initialize the controller\n");
        return;
    }
    DEBUG("[gorm] controller initialized\n");

    /* initializing of host modules */
#ifdef MODULE_GORM_GATT
    gorm_gatt_server_init();
#endif
}

void gorm_ctx_init(gorm_ctx_t *con)
{
    memset(con, 0, sizeof(gorm_ctx_t));
    con->ll.state = GORM_LL_STATE_STANDBY;
    gorm_arch_evt_init(&con->event, con, _on_event);
}

void gorm_app_handler_add(gorm_event_handler_t *handler)
{
    handler->next = _app_handler;
    _app_handler = handler;
}

void gorm_run(void)
{
    gorm_arch_evt_loop();
}

void gorm_notify(gorm_ctx_t *con, uint16_t event)
{
    gorm_arch_evt_post(&con->event, event);
}

// int gorm_adv_start(void)
// {
//     _state = STATE_ADVERTISING;
//     DEBUG("[gorm] adv_start: start advertising\n");

//     /* make sure we are not already advertising */
//     if (_get_by_state(GORM_LL_STATE_ADV) != NULL) {
//         DEBUG("[gorm] adv_start: advertising already in progress\n");
//         return GORM_OK;
//     }

//     /* get an unused context */
//     gorm_ctx_t *con = _get_by_state(GORM_LL_STATE_STANDBY);
//     if (con) {
//         DEBUG("[gorm] adv_start: trigger advertisements on ctx %p\n", con);
//         gorm_ll_adv_conn(&con->ll, gorm_gap_get_default_adv_ctx());
//         return GORM_OK;
//     }

//     return GORM_ERR_CTX_BUSY;
// }

// void gorm_adv_stop(void)
// {
//     _state = STATE_STANDBY;

//     gorm_ctx_t *con = _get_by_state(GORM_LL_STATE_ADV);
//     if (con) {
//         gorm_ll_terminate(&con->ll);
//     }
// }
