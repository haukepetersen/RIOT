/*
 * Copyright (C) 2016 Freie Univerität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/*
 * @ingroup     auto_init_gnrc_netif
 * @{
 *
 * @file
 * @brief       Auto initialization for the CC2538 radio
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#ifdef MODULE_CC2538

#include "board.h"
#include "net/gnrc.h"
#include "net/gnrc/netdev2.h"
#include "net/gnrc/netdev2/ieee802154.h"

#include "cc2538rf.h"

/**
 * @brief   Define stack parameters for the MAC layer thread
 * @{
 */
#define MAC_STACKSIZE       (THREAD_STACKSIZE_DEFAULT)
#define MAC_PRIO            (THREAD_PRIORITY_MAIN - 4)
/** @} */

static cc2538rf_t dev;
static gnrc_netdev2_t gnrc_adpt;
static char stack[MAC_STACKSIZE];

void auto_init_cc2538(void)
{
    cc2538rf_setup(&dev);
    gnrc_netdev2_ieee802154_init(&gnrc_adpt, (netdev2_ieee802154_t *)&dev);
    gnrc_netdev2_init(stack, sizeof(stack), MAC_PRIO, "cc2538rf", &gnrc_adpt);
}

#else
typedef int dont_be_pedantic;
#endif /* MODULE_CC2538 */
