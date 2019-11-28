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

/* we do some ugly device specific multiplexing here for now */
#if defined(MODULE_NRFBLE)
#include "nrfble.h"
#else
#error "[auto_init_gorm] error: now compatible radio driver found"
#endif

/* allocate stack(s) */
#ifdef MODULE_GORM_STANDALONE
static char _host_stack[GORM_CFG_THREAD_STACKSIZE_HOST]

static void *_host_thread(void *arg)
{
    (void)arg;
    gorm_host_run();
    return NULL;
}
#endif

void auto_init_gorm(void)
{
    netdev_t *dev = NULL;

#if defined(MODULE_NRFBLE)
    LOG_DEBUG("[auto_init_gorm] initializing nrfble #0\n");
    dev = nrfble_setup();
#endif

    /* make sure we got a device handle */
    if (!dev) {
        LOG_ERROR("[auto_init_gorm] unable to setup radio for Gorm\n");
        return;
    }

    /* initialize Gorm */
    gorm_init(dev);

    /* if standalone mode is activated, we also spawn Gorm's host thread now */
#ifdef MODULE_GORM_STANDALONE
    LOG_DEBUG("[auto_init_gorm] setting up Gorm's host thread now\n");
    thread_create(_host_stack,
                  sizeof(_host_stack),
                  GORM_CFG_THREAD_PRIO_HOST,
                  THREAD_CREATE_STACKTEST,
                  _host_thread,
                  NULL,
                  GORM_CFG_THREAD_NAME_HOST);
#endif
}
