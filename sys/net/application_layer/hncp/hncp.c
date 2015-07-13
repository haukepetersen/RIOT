/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_dncp
 * @{
 *
 * @file
 * @brief       Home Network Control Protocol (HNCP) Implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "net/hncp.h"
#include "net/dncp.h"
#include "net/hncp/tlvs.h"

static void *event_loop(void *arg)
{
    /* register for incoming UDP packets on target port */

    /* should not be reached */
    return NULL;
}


void hncp_init(ng_nettype_t transport, uint16_t port,
               char *stack, int stacksize, char prio)
{
    /* check transport */

    /* create the HNCP thread */
    thread_create(stack, stacksize, prio, THREAD_CREATE_STACKTEST,
                  event_loop, &port, "hncp");

}
