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
 * @brief       Gorm's link layer thread implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <string.h>

/* TODO: factor out thread flag stuff into something ARCH dependent */
#include "thread_flags.h"

#include "net/gorm/ll.h"
#include "net/gorm/ll/ctrl.h"
#include "net/gorm/l2cap.h"

#define ENABLE_DEBUG                (1)
#include "debug.h"

#define STATE_STANDBY               (0U)
#define STATE_ADVERTISE             (1U)

#define NOTIFY_FLAG                 (0x0001)

#define CTX_NUMOF                   GORM_CFG_CONNECTION_NUMOF

/* internal state, used to keep track of global advertisement status */
static uint8_t _state = STATE_STANDBY;

static thread_t *_host_thread;

/* allocate memory for the defined number of possible connections */
static gorm_ctx_t _ctx[CTX_NUMOF];

static gorm_ctx_t *_get_by_state(uint8_t state)
{
    for (unsigned i = 0; i < CTX_NUMOF; i++) {
        if (_ctx[i].ll.state == state) {
            return &_ctx[i];
        }
    }
    return NULL;
}

static void _on_data(gorm_ctx_t *con, gorm_buf_t *buf)
{
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
            DEBUG("[gorm_ll_host] on_data: invalid LLID value\n");
            gorm_buf_return(buf);
            break;
    }

    DEBUG("~~~~ pkt done, pool size is %u\n", gorm_buf_count(&gorm_buf_pool));
}

void gorm_host_init(void)
{
    DEBUG("[gorm_host] init: setting up the host context\n");

    /* initialize the allocated contexts */
    for (unsigned i = 0; i < CTX_NUMOF; i++) {
        memset(&_ctx[i], 0, sizeof(gorm_ctx_t));
        _ctx[i].ll.state = GORM_LL_STATE_STANDBY;
    }
}

void gorm_host_run(void)
{
    _host_thread = (thread_t *)sched_active_thread;

    DEBUG("[gorm_host] run: running Gorm's host event loop\n");

    while (thread_flags_wait_any(NOTIFY_FLAG)) {
        DEBUG("[gorm_ll_host] run: got notified about incoming data! ~~~~~~~~~~~\n");

        /* TODO: find better and more efficient way to select and dispatch the
         *       the pending data! */
        /* idea: could we limit the max number of connections to 16 and use
         *       dedicated thread flags for each connection? -> dont like the limit*/
        for (unsigned i = 0; i < CTX_NUMOF; i++) {
            gorm_ctx_t *con = &_ctx[i];
            gorm_buf_t *buf;
            while ((buf = gorm_buf_deq(&con->ll.rxq))) {
                _on_data(con, buf);
            }
        }
    }
}

void gorm_host_notify(gorm_ctx_t *con)
{
    (void)con;
    thread_flags_set(_host_thread, NOTIFY_FLAG);
}

void gorm_host_adv_start(void)
{
    _state = STATE_ADVERTISE;
    DEBUG("[gorm_host] adv_start: start advertising\n");

    /* make sure we are not already advertising */
    if (_get_by_state(GORM_LL_STATE_ADV) != NULL) {
        DEBUG("[gorm_host] adv_start: advertising already in progress\n");
        return;
    }

    /* get an unused context */
    gorm_ctx_t *con = _get_by_state(GORM_LL_STATE_STANDBY);
    if (con) {
        DEBUG("[gorm_host] adv_start: trigger advertisements on ctx %p\n", con);
        gorm_ll_adv_conn(&con->ll, gorm_gap_get_default_adv_ctx());
    }
}

void gorm_host_adv_stop(void)
{
    _state = STATE_STANDBY;

    gorm_ctx_t *con = _get_by_state(GORM_LL_STATE_ADV);
    if (con) {
        gorm_ll_adv_stop(&con->ll);
    }
}

void gorm_host_on_con_terminate(void)
{
    if (_state == STATE_ADVERTISE) {
        gorm_host_adv_start();
    }
}
