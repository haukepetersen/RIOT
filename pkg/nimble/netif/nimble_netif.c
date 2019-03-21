/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkt_nimble_gnrc
 *
 * # TODO
 * - many many things...
 *
 *
 * @{
 *
 * @file
 * @brief       NimBLE integration with GNRC
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <limits.h>

#include "assert.h"
#include "thread.h"
#include "mutex.h"

#include "net/ble.h"
#include "net/bluetil/addr.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/pktbuf.h"
#include "net/gnrc/nettype.h"

#include "nimble_netif.h"
#include "nimble_riot.h"
#include "host/ble_gap.h"
#include "host/util/util.h"

#define ENABLE_DEBUG            (0)
#include "debug.h"

#ifdef MODULE_GNRC_SIXLOWPAN
#define NTYPE                   GNRC_NETTYPE_SIXLOWPAN
#else
#define NTYPE                   GNRC_NETTYPE_UNDEF
#endif

/* maximum packet size for IPv6 packets */
#ifndef NIMBLE_NETIF_IPV6_MTU
#define NIMBLE_NETIF_IPV6_MTU    (1280U)    /* as specified in RFC7668 */
#endif

/* buffer configuration
 * - we need one RX buffer per connection
 * - and a single shared TX buffer*/
#define MTU_SIZE                (NIMBLE_NETIF_IPV6_MTU)
#define MBUF_CNT                (MYNEWT_VAL_BLE_MAX_CONNECTIONS + 1)
#define MBUF_OVHD               (sizeof(struct os_mbuf) + \
                                 sizeof(struct os_mbuf_pkthdr))
#define MBUF_SIZE               (MBUF_OVHD + MTU_SIZE)

enum {
    MASTER,
    SLAVE,
};

/* allocate a stack for the netif device */
static char _stack[THREAD_STACKSIZE_DEFAULT];

/* keep the actual device state */
static gnrc_netif_t *_nimble_netif = NULL;
static gnrc_nettype_t _nettype = NTYPE;

/* keep a reference to the event callback */
static nimble_netif_eventcb_t _eventcb;

/* list of open connections and current advertising context */
static clist_node_t _connections;
static nimble_netif_conn_t *_advertising = NULL;
static mutex_t _conn_lock = MUTEX_INIT;

/* allocation of memory for buffering IP packets when handing them to NimBLE */
static os_membuf_t _mem[OS_MEMPOOL_SIZE(MBUF_CNT, MBUF_SIZE)];
static struct os_mempool _mem_pool;
static struct os_mbuf_pool _mbuf_pool;

/* notify the user about state changes for a connection context */
static void _notify(nimble_netif_conn_t *conn, nimble_netif_event_t event)
{
    if (_eventcb) {
        _eventcb(conn, event);
    }
}

static int _match_addr(clist_node_t *node, void *arg)
{
    nimble_netif_conn_t *conn = (nimble_netif_conn_t *)node;
    return (memcmp(conn->addr, arg, BLE_ADDR_LEN) == 0);
}

static int _match_handle(clist_node_t *node, void *arg)
{
    nimble_netif_conn_t *conn = (nimble_netif_conn_t *)node;
    return (conn->handle == *((uint16_t *)arg));
}

static nimble_netif_conn_t *_conn_get_by_addr(const uint8_t *addr)
{
    mutex_lock(&_conn_lock);
    nimble_netif_conn_t *conn;
    conn = (nimble_netif_conn_t *)clist_foreach(&_connections, _match_addr,
                                               (void *)addr);
    mutex_unlock(&_conn_lock);
    return conn;
}

static nimble_netif_conn_t *_conn_get_by_handle(uint16_t handle)
{
    mutex_lock(&_conn_lock);
    nimble_netif_conn_t *conn;
    conn = (nimble_netif_conn_t *)clist_foreach(&_connections, _match_handle,
                                               (void *)&handle);
    mutex_unlock(&_conn_lock);
    return conn;
}

static nimble_netif_conn_t *_conn_find(nimble_netif_conn_t *conn)
{
    mutex_lock(&_conn_lock);
    conn = (nimble_netif_conn_t *)clist_find(&_connections, &conn->node);
    mutex_unlock(&_conn_lock);
    return conn;
}

static nimble_netif_conn_t *_conn_remove(nimble_netif_conn_t *conn)
{
    mutex_lock(&_conn_lock);
    conn = (nimble_netif_conn_t *)clist_remove(&_connections, &conn->node);
    mutex_unlock(&_conn_lock);
    return conn;
}

static void _conn_add(nimble_netif_conn_t *conn)
{
    mutex_lock(&_conn_lock);
    clist_rpush(&_connections, &conn->node);
    mutex_unlock(&_conn_lock);
}

/* copy snip to mbuf */
static struct os_mbuf *_pkt2mbuf(struct os_mbuf_pool *pool, gnrc_pktsnip_t *pkt)
{
    struct os_mbuf *sdu = os_mbuf_get_pkthdr(pool, 0);
    if (sdu == NULL) {
        return NULL;
    }
    while (pkt) {
        int res = os_mbuf_append(sdu, pkt->data, pkt->size);
        if (res != 0) {
            os_mbuf_free_chain(sdu);
            return NULL;
        }
        pkt = pkt->next;
    }
    return sdu;
}

static int _send_pkt(nimble_netif_conn_t *conn, gnrc_pktsnip_t *pkt)
{
    if (conn->coc == NULL) {
        printf("    [] (%p) err: L2CAP not connected (yet)\n", conn);
        return NIMBLE_NETIF_OK;
    }

    struct os_mbuf *sdu = _pkt2mbuf(&_mbuf_pool, pkt);
    if (sdu == NULL) {
        printf("    [] (%p) err: could not alloc mbuf\n", conn);
        return NIMBLE_NETIF_NOMEM;
    }
    int res = ble_l2cap_send(conn->coc, sdu);
    if (res != 0) {
        os_mbuf_free_chain(sdu);
        printf("    [] (%p) err: l2cap send failed (%i)\n", conn, res);
        return NIMBLE_NETIF_DEVERR;
    }

    return NIMBLE_NETIF_OK;
}



static void _netif_init(gnrc_netif_t *netif)
{
    (void)netif;

    DEBUG("[nimg] _netif_init\n");

#ifdef MODULE_GNRC_SIXLOWPAN
    DEBUG("    [] setting max_frag_size to 0\n");
    /* we disable fragmentation for this device, as the L2CAP layer takes care
     * of this */
    _nimble_netif->sixlo.max_frag_size = 0;
#endif
}

static int _netif_send(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    (void)netif;

    // DEBUG("[nimg] _netif_send\n");
    if (pkt->type != GNRC_NETTYPE_NETIF) {
        assert(0);
        return NIMBLE_NETIF_DEVERR;
    }

    gnrc_netif_hdr_t *hdr = (gnrc_netif_hdr_t *)pkt->data;
    /* if packet is bcast or mcast, we send it to every connected node */
    if (hdr->flags &
        (GNRC_NETIF_HDR_FLAGS_BROADCAST | GNRC_NETIF_HDR_FLAGS_MULTICAST)) {
        // DEBUG("    [] sending MULTICAST\n");
        mutex_lock(&_conn_lock);
        clist_node_t *cur = _connections.next;
        if (cur) {
            do {
                cur = cur->next;
                // DEBUG("    [] duplicating packet on conn #%p\n", (void *)cur);
                _send_pkt((nimble_netif_conn_t *)cur, pkt->next);
            } while (cur != _connections.next);
        }
        mutex_unlock(&_conn_lock);
    }
    /* send unicast */
    else {
        // DEBUG("    [] sending UNICAST\n");
        nimble_netif_conn_t *conn = _conn_get_by_addr(
                                            gnrc_netif_hdr_get_dst_addr(hdr));
        if (conn) {
            int res = _send_pkt(conn, pkt->next);
            assert(res == NIMBLE_NETIF_OK);
        }
        else {
            // DEBUG("    [] err: could not find connection to target addr\n");
            // ret =  NIMBLE_NETIF_NOTCONN;
        }
    }

    /* release the packet in GNRC's packet buffer */
    gnrc_pktbuf_release(pkt);
    return 0;
}

/* not used, we pass incoming data to GNRC directly from the NimBLE thread */
static gnrc_pktsnip_t *_netif_recv(gnrc_netif_t *netif)
{
    (void)netif;
    return NULL;
}

static const gnrc_netif_ops_t _nimble_netif_ops = {
    .init = _netif_init,
    .send = _netif_send,
    .recv = _netif_recv,
    .get = gnrc_netif_get_from_netdev,
    .set = gnrc_netif_set_from_netdev,
    .msg_handler = NULL,
};

static inline int _netdev_init(netdev_t *dev)
{
    _nimble_netif = dev->context;

    /* initialize of BLE related buffers */
    if (os_mempool_init(&_mem_pool, MBUF_CNT, MBUF_SIZE, _mem, "nim_gnrc") != 0) {
        return -ENOBUFS;
    }
    if (os_mbuf_pool_init(&_mbuf_pool, &_mem_pool, MBUF_SIZE, MBUF_CNT) != 0) {
        return -ENOBUFS;
    }

    /* get our own address from the controller */
    int res = ble_hs_id_copy_addr(nimble_riot_own_addr_type,
                                  _nimble_netif->l2addr, NULL);
    if (res != 0) {
        return -EINVAL;
    }

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
            memcpy(value, _nimble_netif->l2addr, BLE_ADDR_LEN);
            res = BLE_ADDR_LEN;
            break;
        case NETOPT_ADDR_LEN:
        case NETOPT_SRC_LEN:
            assert(max_len == sizeof(uint16_t));
            *((uint16_t *)value) = BLE_ADDR_LEN;
            res = sizeof(uint16_t);
            break;
        case NETOPT_MAX_PACKET_SIZE:
            assert(max_len >= sizeof(uint16_t));
            *((uint16_t *)value) = MTU_SIZE;
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
            assert(val_len == sizeof(gnrc_nettype_t));
            memcpy(&_nettype, value, sizeof(gnrc_nettype_t));
            res = sizeof(gnrc_nettype_t);
            break;
        default:
            break;
    }

    return res;
}

static const netdev_driver_t _nimble_netdev_driver = {
    .send = NULL,
    .recv = NULL,
    .init = _netdev_init,
    .isr  =  NULL,
    .get  = _netdev_get,
    .set  = _netdev_set,
};

static netdev_t _nimble_netdev_dummy = {
    .driver = &_nimble_netdev_driver,
};





static int _on_data(nimble_netif_conn_t *conn, struct ble_l2cap_event *event)
{
    int ret = 0;
    struct os_mbuf *rxb = event->receive.sdu_rx;
    size_t rx_len = (size_t)OS_MBUF_PKTLEN(rxb);

    // DEBUG("    [] (%p) ON_DATA: received %u bytes\n", conn, rx_len);

    /* allocate netif header */
    gnrc_pktsnip_t *netif_hdr = gnrc_netif_hdr_build(conn->addr, BLE_ADDR_LEN,
                                                     _nimble_netif->l2addr,
                                                     BLE_ADDR_LEN);

    if (netif_hdr == NULL) {
        DEBUG("    [] (%p) err: unable to allocate netif hdr\n", conn);
        ret = NIMBLE_NETIF_NOMEM;
        goto end;
    }

    /* allocate space in the pktbuf to store the packet */
    gnrc_pktsnip_t *payload = gnrc_pktbuf_add(netif_hdr, NULL, rx_len, _nettype);
    if (payload == NULL) {
        DEBUG("    [] (%p) err: unable to allocate payload in pktbuf\n", conn);
        gnrc_pktbuf_release(netif_hdr);
        ret = NIMBLE_NETIF_NOMEM;
        goto end;
    }

    /* copy payload from mbuf into pktbuffer */
    int res = os_mbuf_copydata(rxb, 0, rx_len, payload->data);
    if (res != 0) {
        DEBUG("    [] (%p) err: could not copy data from mbuf chain\n", conn);
        gnrc_pktbuf_release(payload);
        ret = NIMBLE_NETIF_DEVERR;
        goto end;
    }

    /* finally dispatch the receive packet to GNRC */
    // DEBUG("    [] (%p) handing snip of type %i to GNRC\n", conn, (int)payload->type);
    if (!gnrc_netapi_dispatch_receive(payload->type, GNRC_NETREG_DEMUX_CTX_ALL,
                                      payload)) {
        DEBUG("    [] (%p) err: no on interested in the new pkt\n", conn);
        gnrc_pktbuf_release(payload);
    }
    // DEBUG("    [] (%p) GNRC did accept the packet\n", conn);

end:
    /* copy the receive data and free the mbuf */
    os_mbuf_free_chain(rxb);
    /* free the mbuf and allocate a new one for receiving new data */
    rxb = os_mbuf_get_pkthdr(&_mbuf_pool, 0);
    /* due to buffer provisioning, there should always be enough space */
    assert(rxb != NULL);
    ble_l2cap_recv_ready(event->receive.chan, rxb);

    // DEBUG("    [] (%p) done with netif_recv()\n", conn);

    return ret;
}

static void _l2cap_connected(nimble_netif_conn_t *conn,
                             struct ble_l2cap_event *event) {
    assert(conn->coc == NULL);
    conn->coc = event->connect.chan;
    _conn_add(conn);
    DEBUG("    [] (%p) l2cap connected\n", conn);
    _notify(conn, NIMBLE_NETIF_CONNECTED);
}

static int _on_l2cap_client_evt(struct ble_l2cap_event *event, void *arg)
{
    nimble_netif_conn_t *conn = (nimble_netif_conn_t *)arg;
    int res = 0;

    if (event->type != BLE_L2CAP_EVENT_COC_DATA_RECEIVED) {
        DEBUG("[nimg] (%p) _on_l2cap_slave_evt: event %i\n",
              conn, (int)event->type);
    }

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            _l2cap_connected(conn, event);
            break;
        case BLE_L2CAP_EVENT_COC_DISCONNECTED:
            assert(conn->coc);
            conn->coc = NULL;
            break;
        case BLE_L2CAP_EVENT_COC_ACCEPT:
            /* this event should never be triggered for the L2CAP client */
            assert(0);
            break;
        case BLE_L2CAP_EVENT_COC_DATA_RECEIVED:
            res = _on_data(conn, event);
            break;
        default:
            assert(0);
            break;
    }

    return res;
}

static int _on_l2cap_server_evt(struct ble_l2cap_event *event, void *arg)
{
    (void)arg;

    nimble_netif_conn_t *conn;
    if (event->type != BLE_L2CAP_EVENT_COC_DATA_RECEIVED) {
        DEBUG("[nimg] _on_l2cap_server_evt: event %i\n", (int)event->type);
    }

    switch (event->type) {
        case BLE_L2CAP_EVENT_COC_CONNECTED:
            mutex_lock(&_conn_lock);
            conn = _advertising;
            _advertising = NULL;
            mutex_unlock(&_conn_lock);
            DEBUG("    [] (%p) server CONNECTED\n", conn);
            _l2cap_connected(conn, event);
            break;
        case BLE_L2CAP_EVENT_COC_DISCONNECTED:
            conn = _conn_get_by_handle(event->disconnect.conn_handle);
            DEBUG("    [] (%p) server DISCONNECTED\n", conn);
            assert(conn->coc);
            conn->coc = NULL;
            break;
        case BLE_L2CAP_EVENT_COC_ACCEPT: {
            mutex_lock(&_conn_lock);
            conn = _advertising;
            mutex_unlock(&_conn_lock);
            DEBUG("    [] (%p) server ACCEPT\n", conn);
            assert(conn->coc == NULL);
            struct os_mbuf *sdu_rx = os_mbuf_get_pkthdr(&_mbuf_pool, 0);
            /* there should always be enough buffer space */
            assert(sdu_rx != NULL);
            ble_l2cap_recv_ready(event->accept.chan, sdu_rx);
            break;
        }
        case BLE_L2CAP_EVENT_COC_DATA_RECEIVED:
            conn = _conn_get_by_handle(event->receive.conn_handle);
            // DEBUG("    [] (%p) server DATA\n", conn);
            _on_data(conn, event);
            break;
        default:
            assert(0);
            break;

    }

    return 0;
}


static void _on_gap_connected(nimble_netif_conn_t *conn, uint16_t conn_handle)
{
    struct ble_gap_conn_desc desc;
    DEBUG("    [] (%p) getting context for handle %i\n", conn, (int)conn_handle);
    int res = ble_gap_conn_find(conn_handle, &desc);
    assert(res == 0);

    conn->handle = conn_handle;
    memcpy(conn->addr, desc.peer_id_addr.val, BLE_ADDR_LEN);
    DEBUG("    [] (%p) GAP connected to ", conn);
    // bluetil_addr_print(conn->addr);
    DEBUG("\n");
}

static int _on_gap_terminated(nimble_netif_conn_t *conn)
{
    DEBUG("    [] (%p) GAP TERMINATED\n", conn);
    _conn_remove(conn);
    conn->handle = 0;
    if (conn->coc != NULL) {
        DEBUG("    [] (%p) WARNING: GAP terminated but COC != NULL\n", conn);
        conn->coc = NULL;
    }
    mutex_lock(&_conn_lock);
    if (_advertising == conn) {
        DEBUG("    [] (%p) WARNING: terminating advertising context\n", conn);
        _advertising = NULL;
    }
    mutex_unlock(&_conn_lock);
    _notify(conn, NIMBLE_NETIF_DISCONNECTED);
    return 0;
}

static int _on_gap_master_evt(struct ble_gap_event *event, void *arg)
{
    nimble_netif_conn_t *conn = (nimble_netif_conn_t *)arg;
    int res = 0;

    DEBUG("[nimg] (%p) _on_gap_master_evt: event %i\n", conn, (int)event->type);

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT: {
            if (event->connect.status != 0) {
                printf("[nimg] CONNECT error (%i)\n", (int)event->connect.status);
                _notify(conn, NIMBLE_NETIF_ABORT);
                return 1;
            }
            _on_gap_connected(conn, event->connect.conn_handle);
            conn->role = MASTER;

            struct os_mbuf *sdu_rx = os_mbuf_get_pkthdr(&_mbuf_pool, 0);
            /* we should never run out of buffer space... */
            assert(sdu_rx != NULL);

            DEBUG("    [] (%p) calling l2cap connect\n", conn);
            res = ble_l2cap_connect(event->connect.conn_handle,
                                    NIMBLE_NETIF_CID, MTU_SIZE, sdu_rx,
                                    _on_l2cap_client_evt, conn);
            if (res != 0) {
                os_mbuf_free_chain(sdu_rx);
                DEBUG("    [] (%p) l2cap connect: FAIL (%i)\n", conn, res);
            }
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT:
            printf("    [] (%p) GAP master disconnect (%i)\n", conn,
                   event->disconnect.reason);
            DEBUG("    [] (%p) -> role %i, addr ", conn, (int)event->disconnect.conn.role);
            // bluetil_addr_print(event->disconnect.conn.peer_id_addr.val);
            DEBUG("\n");
            res = _on_gap_terminated(conn);
            break;
        default:
            DEBUG("    [] (%p) ERROR: unknown GAP event!\n", conn);
            assert(0);
            break;
    }

    return res;
}

static int _on_gap_slave_evt(struct ble_gap_event *event, void *arg)
{
    nimble_netif_conn_t *conn = (nimble_netif_conn_t *)arg;
    int res = 0;

    DEBUG("[nimg] (%p) _on_gap_slave_evt: event %i\n", conn, (int)event->type);

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT: {
            if (event->connect.status != 0) {
                printf("[nimg] CONNECT error (%i)\n", (int)event->connect.status);
                _notify(conn, NIMBLE_NETIF_ABORT);
                return 1;
            }
            _on_gap_connected(conn, event->connect.conn_handle);
            conn->role = SLAVE;
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT:
            printf("    [] (%p) GAP slave disconnect (%i)\n", conn,
                   event->disconnect.reason);
            DEBUG("    [] (%p) -> role %i, addr ", conn, (int)event->disconnect.conn.role);
            // bluetil_addr_print(event->disconnect.conn.peer_id_addr.val);
            DEBUG("\n");
            res = _on_gap_terminated(conn);
            break;
        default:
            DEBUG("    [] (%p) ERROR: unknown GAP event!\n", conn);
            assert(0);
            break;
    }

    return res;
}

int nimble_netif_connect(nimble_netif_conn_t *conn,
                        const ble_addr_t *addr,
                        const struct ble_gap_conn_params *conn_params,
                        uint32_t connect_timeout)
{
    assert(conn);
    assert(addr);
    assert(_eventcb);

    printf("[nimg] (%p) netif_connect: to ", conn);
    // bluetil_addr_print(addr->val);
    printf(" (%p)\n", (void *)conn);

    if ((conn->handle != 0) || (conn->coc != NULL)) {
        printf("    [] (%p) ERROR: connection is busy\n", conn);
        return NIMBLE_NETIF_BUSY;
    }
    /* check that there is no open connection with the given addr */
    if (_conn_get_by_addr(addr->val) != NULL) {
        printf("    [] (%p) ERROR: already connected to that address\n", conn);
        return NIMBLE_NETIF_ALREADY;
    }

    int res = ble_gap_connect(nimble_riot_own_addr_type, addr, connect_timeout,
                              conn_params, _on_gap_master_evt, conn);
    if (res != 0) {
        printf("    [] (%p) ERROR: gap_connect failed (%i)\n", conn, res);
        return NIMBLE_NETIF_DEVERR;
    }

    printf("    [] (%p) OK\n", conn);
    return NIMBLE_NETIF_OK;
}

int nimble_netif_accept(nimble_netif_conn_t *conn,
                       const uint8_t *ad, size_t ad_len,
                       const struct ble_gap_adv_params *adv_params)
{
    assert(conn);
    assert(adv_params);

    int res;

    DEBUG("[nimg] (%p) netif_accept: start advertising\n", (void *)conn);

    if ((_advertising != NULL) || (conn->handle != 0) || (conn->coc != NULL)) {
        DEBUG("    [] (%p) ERROR: busy (advertising is %p)\n", conn, _advertising);
        return NIMBLE_NETIF_BUSY;
    }

    /* configure advertising data */
    if (ad != NULL) {
        res = ble_gap_adv_set_data(ad, (int)ad_len);
        if (res != 0) {
            printf("    [] (%p) ERROR: unable to set AD (%i)\n", conn, res);
            return NIMBLE_NETIF_DEVERR;
        }
    }
    /* start advertising the device */
    mutex_lock(&_conn_lock);
    _advertising = conn;
    mutex_unlock(&_conn_lock);
    res = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                            adv_params, _on_gap_slave_evt, _advertising);
    if (res != 0) {
        printf("    [] (%p) ERROR: unable to start advertising (%i)\n", conn, res);
        return NIMBLE_NETIF_DEVERR;
    }

    DEBUG("    [] (%p) OK\n", conn);
    return NIMBLE_NETIF_OK;
}

int nimble_netif_terminate(nimble_netif_conn_t *conn)
{
    assert(conn);

    DEBUG("[nimg] terminate (%p)\n", (void *)conn);

    if (conn == _advertising) {
        ble_gap_adv_stop();
        mutex_lock(&_conn_lock);
        _advertising = NULL;
        mutex_unlock(&_conn_lock);
        DEBUG("    [] was advertising\n");
        return NIMBLE_NETIF_OK;
    }

    if (_conn_find(conn) == conn) {
        int res = ble_gap_terminate(ble_l2cap_get_conn_handle(conn->coc),
                                    BLE_ERR_REM_USER_CONN_TERM);
        if (res != 0) {
            DEBUG("    [] ERROR: triggering termination (%i)\n", res);
            return NIMBLE_NETIF_DEVERR;
        }
        DEBUG("    [] OK\n");
        return NIMBLE_NETIF_OK;
    }

    DEBUG("    [] ERROR: not connected\n");
    return NIMBLE_NETIF_NOTCONN;
}


int nimble_netif_is_connected(nimble_netif_conn_t *conn)
{
    return _conn_find(conn) != NULL;
}

nimble_netif_conn_t *nimble_netif_get_conn(const uint8_t *addr)
{
    return _conn_get_by_addr(addr);
}

nimble_netif_conn_t *nimble_netif_get_adv_ctx(void)
{
    return _advertising;
}

static int _conn_dump(clist_node_t *node, void *arg)
{
    (void)arg;
    char roles[2] = { 'M', 'S' };
    nimble_netif_conn_t *conn = (nimble_netif_conn_t *)node;
    printf("- (%c) ", roles[conn->role]);
    bluetil_addr_print(conn->addr);
    printf(" -> ");
    bluetil_addr_print_ipv6_iid(conn->addr);
    printf("\n");
    return 0;
}

void nimble_netif_print_conn_info(void)
{
    mutex_lock(&_conn_lock);
    clist_foreach(&_connections, _conn_dump, NULL);
    mutex_unlock(&_conn_lock);
}

void nimble_netif_init(void)
{
    DEBUG("[nimg] init()\n");

    DEBUG("    [] creating L2CAP server\n");
    int res = ble_l2cap_create_server(BLE_L2CAP_CID_IPSP, MTU_SIZE,
                                      _on_l2cap_server_evt, NULL);
    if (res != 0) {
        DEBUG("    [] ERROR: failed to create L2CAP server (%i)\n", res);
    }

    gnrc_netif_create(_stack, sizeof(_stack), GNRC_NETIF_PRIO,
                      "nimble_netif", &_nimble_netdev_dummy, &_nimble_netif_ops);
}

int nimble_netif_eventcb(nimble_netif_eventcb_t cb)
{
    _eventcb = cb;
    return NIMBLE_NETIF_OK;
}
