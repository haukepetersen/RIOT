/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_facemaster_faceimp_sockudp
 *
 * # TODO
 * - many many things...
 *
 *
 * @{
 *
 * @file
 * @brief       Implementation of netdev-based IEEE802.15.4 based faceimp
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "log.h"
#include "assert.h"

static faceimp_sockudp_eventcb_t _eventcb = NULL;

static clist_node_t _faces;

// TODO: (pre)allocate imps?! and what about list nodes?!

static int _send(faceimp_t *imp, void *data, size_t len)
{
    // TODO: assert imp type somehow... (maybe static field in struct?!)
    faceimp_sockudp_t *sockimp = (faceimp_sockudp_t *)imp;

}

static void *_handler_thread(void *arg)
{
    (void)arg;

    while (1) {
        sock_udp_recv(...);


    }

    /* should never be reached */
    return NULL;
}



int faceimp_sockudp_init(const faceimp_sockudp_params_t *params)
{
    // create (and open?) UDP socket
    assert(params);



    return FACEIMP_OK;
}

// TODO: factor out the opening of a specific port -> support for multiple
//       'virtual devices (ports)'?!

int faceimp_sockudp_eventcb(faceimp_sockudp_eventcb_t cb)
{
    _eventcb = cb;
}

int facemimp_sockudp_connect(faceimp_sockudp_t *sockimp, sock_udp_ep_t *ep)
{
    assert(sockimp);
    assert(ep);

    clist_rpush(&_faces, &sockimp->listnode);

    // TODO
    // notify user using eventcb

    return FACEIMP_OK;
}

int faceimp_sockudp_terminate(faceimp_sockudp_t *sockimp)
{
    assert(sockimp);

    // TODO: remove sock_udp_ep_t in sockimp from list of open connections

    return FACEIMP_OK;
}
