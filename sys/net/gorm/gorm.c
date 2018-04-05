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


#include "net/gorm.h"
#include "net/gorm/ll.h"
#include "net/gorm/ll/host.h"

#ifdef MODULE_GORM_PERIPHERAL
#include "net/gorm/gatt.h"
#endif

#define ENABLE_DEBUG    (1)
#include "debug.h"

void gorm_host_init(void)
{
    DEBUG("[gorm] initializing the host\n");

#ifdef MODULE_GORM_PERIPHERAL
    gorm_gatt_server_init();
#endif
}

void gorm_host_run(void)
{
    DEBUG("[gorm] running the host\n");

#ifdef MODULE_GORM_PERIPHERAL
    DEBUG("[gorm] starting to advertise\n");
    /* start advertising GAP data */
    /* TODO: move this */
    gorm_ll_periph_adv_start();
#endif

    /* TODO: merge functions ... */
    DEBUG("[gorm] running the host part of the link layer now\n");
    gorm_ll_host_run();
}

void gorm_run_controller(netdev_t *radio)
{
    gorm_ll_run(radio);
}
