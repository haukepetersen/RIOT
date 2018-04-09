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
 * @brief       Gorm's radio wrapper
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_LL_TRX_H
#define NET_GORM_LL_TRX_H

#include "net/netdev/ble.h"
#include "net/gorm/buf.h"

typedef int(*gorm_ll_trx_cb_t)(gorm_buf_t *buf, void *arg);

/**
 * @brief   Initialize Gorm's radio wrapper
 *
 * @param[in] dev   network device to use
 *
 * @return  GORM_OK on success
 * @return  GORM_ERR_RADIO on failed radio initialization
 */
int gorm_ll_trx_init(netdev_t *dev);

/**
 * @brief   Cancel any ongoing operation and put the radio into idle mode
 */
void gorm_ll_trx_stop(void);


void gorm_ll_trx_send(gorm_buf_t *buf, netdev_ble_ctx_t *ctx,
                      gorm_ll_trx_cb_t cb, void *arg);

void gorm_ll_trx_recv(gorm_buf_t *buf, netdev_ble_ctx_t *ctx,
                      gorm_ll_trx_cb_t cb, void *arg);

void gorm_ll_trx_send_next(gorm_buf_t *buf, gorm_ll_trx_cb_t cb);

void gorm_ll_trx_recv_next(gorm_buf_t *buf, gorm_ll_trx_cb_t cb);

#endif /* NET_GORM_LL_TRX_H */
/** @} */
