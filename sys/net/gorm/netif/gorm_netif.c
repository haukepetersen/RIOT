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

#include "net/gorm.h"
#include "net/gorm/coc.h"

#define ENABLE_DEBUG            (1)
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
static gnrc_netif_t *_netif = NULL;
static gnrc_nettype_t _nettype = NETTYPE;

static gorm_coc_t _coc[GORM_NETIF_CONCNT];


static void _netif_init(gnrc_netif_t *netif)
{
    gnrc_netif_default_init(netif);
}

static int _netif_send(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    assert(pkt->type == GNRC_NETTYPE_NETIF);
    (void)netif;

    // TODO error value....
    int res = 0;
    gnrc_netif_hdr_t *hdr = (gnrc_netif_hdr_t *)pkt->data;
    uint8_t *dst_addr = gnrc_netif_hdr_get_dst_addr(hdr);

        /* if packet is bcast or mcast, we send it to every connected node */
    for (unsigned i = 0; i < ARRAY_SIZE(_coc); i++) {
        if (_coc[i].con != NULL) {
            if (hdr->flags & FLAG_BCAST) {
                res = gorm_coc_send(&_coc[i], (iolist_t *)pkt->next);
            }
            else if (memcmp(_coc[i].con->ll.peer_addr, dst_addr, BLE_ADDR_LEN) == 0) {
                res = gorm_coc_send(&_coc[i], (iolist_t *)pkt->next);
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

/* not used, we pass incoming data directly from the Gorm thread */
static gnrc_pktsnip_t *_netif_recv(gnrc_netif_t *netif)
{
    (void)netif;
    return NULL;
}

static int _netdev_init(netdev_t *dev)
{
    _netif = dev->context;
    return 0;
}

static inline int _netdev_get(netdev_t *dev, netopt_t opt,
                              void *value, size_t max_len)
{
    (void)dev;
    int res = -ENOTSUP;

    switch (opt) {
        case NETOPT_ADDRESS:
            assert(max_len >= BLE_ADDR_LEN);
            memcpy(value, _netif->l2addr, BLE_ADDR_LEN);
            res = BLE_ADDR_LEN;
            break;
        case NETOPT_ADDR_LEN:
        case NETOPT_SRC_LEN:
            assert(max_len == sizeof(uint16_t));
            *((uint16_t *)value) = BLE_ADDR_LEN;
            res = sizeof(uint16_t);
            break;
        case NETOPT_MAX_PDU_SIZE:
            assert(max_len >= sizeof(uint16_t));
            *((uint16_t *)value) = GORM_NETIF_MTU;
            res = sizeof(uint16_t);
            break;
        case NETOPT_PROTO:
            assert(max_len == sizeof(gnrc_nettype_t));
            *((gnrc_nettype_t *)value) = _nettype;
            res = sizeof(gnrc_nettype_t);
            break;
        case NETOPT_DEVICE_TYPE:
            assert(max_len == sizeof(uint16_t));
            *((uint16_t *)value) = NETDEV_TYPE_BLE;
            res = sizeof(uint16_t);
            break;
        default:
            break;
    }

    return res;
}

static inline int _netdev_set(netdev_t *dev, netopt_t opt,
                              const void *value, size_t val_len)
{
    (void)dev;
    int res = -ENOTSUP;

    switch (opt) {
        case NETOPT_PROTO:
            assert(val_len == sizeof(_nettype));
            memcpy(&_nettype, value, sizeof(_nettype));
            res = sizeof(_nettype);
            break;
        default:
            break;
    }

    return res;
}

static const gnrc_netif_ops_t _netif_ops = {
    .init = _netif_init,
    .send = _netif_send,
    .recv = _netif_recv,
    .get = gnrc_netif_get_from_netdev,
    .set = gnrc_netif_set_from_netdev,
    .msg_handler = NULL,
};

static const netdev_driver_t _netdev_driver = {
    .send = NULL,
    .recv = NULL,
    .init = _netdev_init,
    .isr  =  NULL,
    .get  = _netdev_get,
    .set  = _netdev_set,
};

static netdev_t _netdev_dummy = {
    .driver = &_netdev_driver,
};

static void _on_coc_pkt(gorm_coc_t *coc)
{
    assert(coc);

    /* allocate netif header */
    gnrc_pktsnip_t *hsnip = gnrc_netif_hdr_build(coc->con->ll.addr, BLE_ADDR_LEN,
                                                 _netif->l2addr, BLE_ADDR_LEN);
    if (hsnip == NULL) {
        return;
    }
    /* we need to add the device PID to the netif header */
    gnrc_netif_hdr_t *netif_hdr = (gnrc_netif_hdr_t *)hsnip->data;
    netif_hdr->if_pid = _nimble_netif->pid;

    /* allocate space in the pktbuf and store the packet */
    gnrc_pktsnip_t *psnip = gnrc_pktbuf_add(hsnip, NULL, coc->rx_len, _nettype);
    if (psnip == NULL) {
        gnrc_pktbuf_release(phnip);
        return;
    }
    gorm_coc_copy_flat(coc, psnip->data);

    /* dispatch the received packet to GNRC */
    if (!gnrc_netapi_dispatch_receive(psnip->type, GNRC_NETREG_DEMUX_CTX_ALL,
                                      psnip)) {
        gnrc_pktbuf_release(psnip);
    }
}

int gorm_netif_init(void)
{
    memset(_coc, 0, sizeof(_coc));
    gnrc_netif_create(_stack, sizeof(_stack), GNRC_NETIF_PRIO,
                      "gorm_netif", &_netdev_dummy, &_netif_ops);

    _on_coc_pkt(NULL);

    return GORM_OK;
}
