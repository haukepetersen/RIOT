/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm_netif
 * @{
 *
 * @file
 * @brief       GNRC netif adaption for Gorm
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "assert.h"

#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/pktbuf.h"
#include "net/gnrc/nettype.h"

#define ENABLE_DEBUG            (0)
#include "debug.h"

#ifdef MODULE_GNRC_SIXLOWPAN
#define NETTYPE                 GNRC_NETTYPE_SIXLOWPAN
#elif defined(MODULE_GNRC_IPV6)
#define NETTYPE                 GNRC_NETTYPE_IPV6
#else
#define NETTYPE                 GNRC_NETTYPE_UNDEF
#endif

#define FLAG_BCAST              (GNRC_NETIF_HDR_FLAGS_BROADCAST \
                                 | GNRC_NETIF_HDR_FLAGS_MULTICAST)

static char _stack[THREAD_STACKSIZE_DEFAULT];
static gnrc_nettype_t _nettype = NETTYPE;

static gorm_coc_t *_coc[GORM_NETIF_CONCNT];


static int _send(gorm_coc_t *coc, gnrc_pktsnip_t *pkt)
{
    assert(coc);
    assert(pkt);

    return
}


static void _netif_init(gnrc_netif_t *netif)
{
    gnrc_netif_default_init(netif);
}

static int _netif_send(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    assert(pkt->type == GNRC_NETTYPE_NETIF);

    int res;
    gnrc_netif_hdr_t *hdr = (gnrc_netif_hdr_t *)pkt->data;
    uint8_t *dst_addr = gnrc_netif_hdr_get_dst_addr(hdr);

        /* if packet is bcast or mcast, we send it to every connected node */
    for (unsigned i = 0; i < ARRAY_SIZE(_coc); i++) {
        if (_coc[i].con != NULL) {
            if (hdr->flags & FLAG_BCAST) {
                res = gorm_coc_send(_coc[i], (iotlist_t *)pkt->next);
            }
            else if (memcmp(_coc[i].con->ll.peer_addr, dst_addr, BLE_ADDR_LEN) == 0) {
                res = gorm_coc_send(_coc[i], (iotlist_t *)pkt->next);
                break;
            }
            else {
                res = GORM_ERR_NOTCON;
                break;
            }
        }
    }

    /* release the packet in GNRC's packet buffer */
    gnrc_pktbuf_release(pkt);
    return res;
}

static

static int _netdev_init(netdev_t *dev)
{
    _nimble_netif = dev->context;
}
