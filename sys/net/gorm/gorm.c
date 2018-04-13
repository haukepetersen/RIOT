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

#include "log.h"

#include "net/gorm.h"
#include "net/gorm/ll.h"
#include "net/gorm/host.h"

#ifdef MODULE_GORM_GATT
#include "net/gorm/gatt.h"
#endif

#define ENABLE_DEBUG    (1)
#include "debug.h"

/* allocate initial buffer memory */
static char _buf_pool[GORM_CFG_POOLSIZE * sizeof(gorm_buf_t)];

void gorm_init(netdev_t *dev)
{
    /* initialize buffer pool */
    gorm_buf_addmem(_buf_pool, sizeof(_buf_pool));
    DEBUG("[gorm] initialized buffer pool\n");

    /* initializing the controller */
    if (gorm_ll_controller_init(dev) != GORM_OK) {
        LOG_ERROR("[gorm] error: unable to initialize the controller\n");
        return;
    }
    DEBUG("[gorm] controller initialized\n");

    /* initializing of host modules */
#ifdef MODULE_GORM_GATT
    gorm_gatt_server_init();
#endif

    gorm_ll_host_init();
}
