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
#include "net/gorm/pdupool.h"

/* we do some ugly device specific multiplexing here for now */
#if defined(MODULE_NRFBLE)
#include "nrfble.h"
#elif defined(MODULE_SOME_BLE_RADIO_DRIVER)
#include "somedrv.h"
#include "somedrv_params.h"
static somedrv_t somedrv_dev;
#else
#error "[auto_init_gorm] error: now compatible radio driver found"
#endif

/* set CREATE_STACKTEST flag depending on log level */
#if (LOG_LEVEL >= LOG_DEBUG)
#define TFLAGS              THREAD_CREATE_STACKTEST
#else
#define TFLAGS              (0)
#endif

/* allocate stack(s) */
static char stack_ctrl[GORM_CFG_THREAD_STACKSIZE_CONTROLLER];
#ifdef MODULE_GORM_STANDALONE
static char stack_host[GORM_CFG_THREAD_STACKSIZE_HOST]
#endif

/* TODO: move together with the pool mem allocation below */
static char pool[GORM_CFG_POOLSIZE * sizeof(gorm_buf_t)];

static netdev_t *radio_setup(void)
{
    netdev_t *dev = NULL;

#if defined(MODULE_NRFBLE)
    LOG_DEBUG("[auto_init_gorm] initializing nrfble #0\n");
    dev = nrfble_setup();
#elif defined(MODULE_SOME_BLE_RADIO_DRIVER)
    LOG_DEBUG("[auto_init_gorm] initializing somedrv #0\n");
    somedrv_setup(&somedrv_dev, &somedrv_params[0]);
    dev = (netdev_t *)(&somedrv_dev);
#endif

    return dev;
}

static void *ctrl_thread(void *arg)
{
    netdev_t *dev = (netdev_t *)arg;
    gorm_run_controller(dev);
    return NULL;
}

#ifdef MODULE_GORM_STANDALONE
static void *host_thread(void *arg)
{
    (void)arg;
    gorm_host_run();
    return NULL;
}
#endif

void auto_init_gorm(void)
{
    netdev_t *dev = radio_setup();

    /* make sure we got a device handle */
    if (!dev) {
        LOG_ERROR("[auto_init_gorm] unable to setup radio for Gorm\n");
        return;
    }

    /* allocate the PDU pool */
    /* TODO: move somewhere better suited */
    gorm_pdupool_addmem(pool, sizeof(pool));

    /* initialize the host */
    gorm_host_init();

    /* run Gorm's link layer */
    LOG_DEBUG("[auto_init_gorm] setting up Gorm's link layer thread now\n");
    thread_create(stack_ctrl,
                  sizeof(stack_ctrl),
                  GORM_CFG_THREAD_PRIO_CONTROLLER,
                  TFLAGS,
                  ctrl_thread,
                  dev,
                  GORM_CFG_THREAD_NAME_CONTROLLER);

    /* if standalone mode is activated, we also spawn Gorm's host thread now */
#ifdef MODULE_GORM_STANDALONE
    LOG_DEBUG("[auto_init_gorm] setting up Gorm's host thread now\n");
    thread_create(stack_host,
                  sizeof(stack_host),
                  GORM_CFG_THREAD_PRIO_HOST,
                  TFLAGS,
                  host_thread,
                  NULL,
                  GORM_CFG_THREAD_NAME_HOST);
#endif
}
