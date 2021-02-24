/*
 * Copyright (C) 2020-2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_netif GNRC netdif wrapper for Gorm
 * @ingroup     net_gorm
 * @brief       GNRC netif implementation for Gorm
 * @{
 *
 * @file
 * @brief       Gorm's GNRC netif wrapper
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_NETIF_H
#define NET_GORM_NETIF_H

#include "net/gorm.h"
#include "net/gorm/gap.h"

#ifdef __cplusplus
extern "C" {
#endif


int gorm_netif_init(void);
int gorm_netif_accept(gorm_gap_adv_ctx_t *adv_ctx);
int gorm_netif_connect(gorm_gap_conn_ctx_t *con_ctx);

void gorm_netif_dump_state(void);


#ifdef __cplusplus
}
#endif

#endif /* NET_GORM_NETIF_H */
