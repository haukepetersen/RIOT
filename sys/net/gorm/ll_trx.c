/*
 * Copyright (C) 2018 Freie Universität Berlin
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
 * @brief       Gorm's radio wrapper implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "assert.h"

#include "net/gorm/ll/trx.h"

static netdev_t *radio;

static gorm_buf_t *trx_buf;
static gorm_ll_trx_cb_t trx_cb;
static void *trx_arg;

static void on_trx(netdev_t *netdev, netdev_event_t event)
{
    (void)netdev;
    (void)event;

    int done = 1;
    if (trx_cb) {
        done = trx_cb(trx_buf, trx_arg);
    }
    if (done) {
        gorm_ll_trx_stop();
    }
}

static void remember(gorm_buf_t *buf, gorm_ll_trx_cb_t cb)
{
    trx_buf = buf;
    trx_cb = cb;
}

void gorm_ll_trx_send(gorm_buf_t *buf, netdev_ble_ctx_t *ctx,
                      gorm_ll_trx_cb_t cb, void *arg)
{
    assert(buf && ctx);

    remember(buf, cb);
    trx_arg = arg;
    netdev_ble_set_ctx(radio, ctx);
    netdev_ble_send(radio, &buf->pkt);
}

void gorm_ll_trx_recv(gorm_buf_t *buf, netdev_ble_ctx_t *ctx,
                      gorm_ll_trx_cb_t cb, void *arg)
{
    assert(buf && ctx && cb);

    remember(buf, cb);
    trx_arg = arg;
    netdev_ble_set_ctx(radio, ctx);
    netdev_ble_recv(radio, &buf->pkt);
}

void gorm_ll_trx_send_next(gorm_buf_t *buf, gorm_ll_trx_cb_t cb)
{
    remember(buf, cb);
    netdev_ble_send(radio, &buf->pkt);
}

void gorm_ll_trx_recv_next(gorm_buf_t *buf, gorm_ll_trx_cb_t cb)
{
    remember(buf, cb);
    netdev_ble_recv(radio, &buf->pkt);
}

void gorm_ll_trx_stop(void)
{
    netdev_ble_stop(radio);
}

int gorm_ll_trx_init(netdev_t *dev)
{
    assert(dev);

    /* setup the radio */
    radio = dev;
    radio->event_callback = on_trx;
    return radio->driver->init(radio);
}
