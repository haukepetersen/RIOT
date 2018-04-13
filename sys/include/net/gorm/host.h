/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm
 * @brief
 * @{
 *
 * @file
 * @brief       Gorm's link layer part that runs in the host thread
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_HOST_H
#define NET_GORM_HOST_H

#include "net/gorm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Event types send from the controller to the host
 * @{
 */
#define GORM_HOST_EVT_DATA              (0x0001)
#define GORM_HOST_EVT_CONNECTED         (0x0002)
#define GORM_HOST_EVT_CON_ABORT         (0x0004)
#define GORM_HOST_EVT_CON_TIMEOUT       (0x0008)
#define GORM_HOST_EVT_CON_CLOSED        (0x0010)
/** @} */

void gorm_host_init(void);

void gorm_host_run(void);

void gorm_host_notify(gorm_ctx_t *con, int event);

void gorm_host_adv_stop(void);
void gorm_host_adv_start(void);

#ifdef __cplusplus
}
#endif

#endif /* NET_GORM_HOST_H */
/** @} */
