/*
 * Copyright (C) 2017 Freie Universität Berlin
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

#include "log.h"
#include "assert.h"
#include "thread.h"

#include "net/gorm.h"

#if IS_ACTIVE(MODULE_GORM_NETIF)
#include "net/gorm/netif.h"
#endif

/* we do some ugly device specific multiplexing here for now */
#if defined(MODULE_NRFBLE)
#include "nrfble.h"
#else
#error "[auto_init_gorm] error: now compatible radio driver found"
#endif

/* allocate stack(s) */
// TODO: re-introduce standalone module?!
// #ifdef MODULE_GORM_STANDALONE
static char _stack[CONFIG_GORM_THREAD_STACKSIZE];

static void *_gorm_thread(void *arg)
{
    (void)arg;
    gorm_run();
    return NULL;
}
// #endif

void auto_init_gorm(void)
{
    LOG_DEBUG("[auto_init_gorm] initializing nrfble #0\n");
    netdev_t *dev = nrfble_setup();

    /* initialize Gorm */
    gorm_init(dev);

    /* if standalone mode is activated, we also spawn Gorm's host thread now */
// #ifdef MODULE_GORM_STANDALONE
    LOG_DEBUG("[auto_init_gorm] setting up Gorm's thread now\n");
    thread_create(_stack, sizeof(_stack),
                  CONFIG_GORM_THREAD_PRIO, THREAD_CREATE_STACKTEST,
                  _gorm_thread, NULL, "gorm");

    if (IS_ACTIVE(MODULE_GORM_NETIF)) {
        gorm_netif_init();
    }

// #endif
}
