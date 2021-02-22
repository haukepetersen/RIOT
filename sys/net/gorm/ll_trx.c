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

#include "net/gorm/ll/trx.h"

static netdev_t *_radio;

static gorm_buf_t *_trx_buf;
static gorm_ll_trx_cb_t _trx_cb;
static gorm_ll_ctx_t *_trx_ctx;

static void on_trx(netdev_t *netdev, netdev_event_t event)
{
    (void)netdev;
    (void)event;

    int done = 1;
    if (_trx_cb) {
        done = _trx_cb(_trx_buf, _trx_ctx);
    }
    if (done) {
        netdev_ble_terminate(_radio);
    }
}

static void _remember(gorm_buf_t *buf, gorm_ll_trx_cb_t cb)
{
    _trx_buf = buf;
    _trx_cb = cb;
}

void gorm_ll_trx_send(gorm_buf_t *buf, netdev_ble_ctx_t *ctx,
                      gorm_ll_trx_cb_t cb, gorm_ll_ctx_t *ll_ctx)
{
    _remember(buf, cb);
    _trx_ctx = ll_ctx;
    netdev_ble_set_ctx(_radio, ctx);
    netdev_ble_send(_radio, &buf->pkt);
}

void gorm_ll_trx_recv(gorm_buf_t *buf, netdev_ble_ctx_t *ctx,
                      gorm_ll_trx_cb_t cb, gorm_ll_ctx_t *ll_ctx)
{
    _remember(buf, cb);
    _trx_ctx = ll_ctx;
    netdev_ble_set_ctx(_radio, ctx);
    netdev_ble_recv(_radio, &buf->pkt);
}

void gorm_ll_trx_send_next(gorm_buf_t *buf, gorm_ll_trx_cb_t cb)
{
    _remember(buf, cb);
    netdev_ble_send(_radio, &buf->pkt);
}

void gorm_ll_trx_recv_next(gorm_buf_t *buf, gorm_ll_trx_cb_t cb)
{
    _remember(buf, cb);
    netdev_ble_recv(_radio, &buf->pkt);
}

void gorm_ll_trx_stop(gorm_ll_ctx_t *ll_ctx)
{
    if (ll_ctx == _trx_ctx) {
        netdev_ble_stop(_radio);
    }
}

void gorm_ll_trx_terminate(gorm_ll_ctx_t *ll_ctx)
{
    if (ll_ctx == _trx_ctx) {
        netdev_ble_terminate(_radio);
    }
}

int gorm_ll_trx_init(netdev_t *dev)
{
    assert(dev);

    /* setup the radio */
    _radio = dev;
    _radio->event_callback = on_trx;
    return _radio->driver->init(_radio);
}
