/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm_l2cap_coc
 * @{
 *
 * @file
 * @brief       Gorm's L2CAP connection oriented channel (COC) implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <errno.h>

#include "net/gorm.h"
#include "net/gorm/l2cap.h"
#include "net/gorm/util.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

// TODO: substract L2CAP (data) header len from this?
// TODO: subtract also all BLE headers...
#define OWN_MTU         NETDEV_BLE_PDU_MAXLEN

// static void _notify(gorm_coc_t *coc, unsigned event, void *val)
// {
//     /* TODO */
//     (void)coc;
//     (void)event;
//     (void)val;
// }

static gorm_coc_t *_find_coc(gorm_ctx_t *con, uint16_t cid)
{
    gorm_coc_t *tmp = con->l2cap.cocs;
    while (tmp != NULL) {
        if (tmp->cid_own == cid) {
            return tmp;
        }
    }
    return NULL;
}

static gorm_coc_t *_find_coc_by_psm(gorm_ctx_t *con, uint16_t psm)
{
    gorm_coc_t *tmp = con->l2cap.cocs;
    while (tmp != NULL) {
        if (tmp->psm == psm) {
            return tmp;
        }
    }
    return NULL;
}

void gorm_coc_on_data(gorm_ctx_t *con, gorm_buf_t *buf,
                      uint16_t cid, uint8_t llid,
                      uint8_t *data, size_t data_len)
{
    gorm_coc_t *coc = _find_coc(con, cid);
    if (coc == NULL) {
        /* TODO: send appropriate l2cap return message?! */
        DEBUG("[gorm_l2cap] _find_cid: data on invalid channel 0x%04x\n", (int)cid);
        gorm_buf_return(buf);
        return;
    }

    /* if we get the a first fragment */
    if (llid == BLE_LL_LLID_DATA_START) {
        /* data must hold at least 2 data bytes (SDU field), if not we drop */
        if (data_len < 2) {
            gorm_buf_return(buf);
            return;
        }

        // TODO: if a incomplete packet is in the queue, drop it? */
        if (coc->rx_len != 0) {
            // drop old shit...
            gorm_buf_return_q(&coc->rxq);
        }

        coc->rx_len = gorm_util_letohs(data);
        coc->rx_pos = (data_len - 2);
    }
    else {          /* must be BLE_LL_LLID_DATA_CONT then */
        /* if rx_len is 0, drop packet, as we are not expecting any continuation */
        if (coc->rx_len == 0) {
            gorm_buf_return(buf);
            return;
        }

        coc->rx_pos += data_len;
    }

    gorm_buf_enq(&coc->rxq, buf);
    if (coc->rx_pos >= coc->rx_len) {
        coc->on_data(coc);
        gorm_buf_return_q(&coc->rxq);
        coc->rx_len = 0;
    }
}

void gorm_coc_read_to_flat(gorm_coc_t *coc, void *buf)
{
    assert(coc);
    assert(buf);

    if ((coc->rx_len == 0) || (coc->rx_len > coc->rx_pos)) {
        return;
    }

    gorm_buf_t *chunk = gorm_buf_deq(&coc->rxq);
    size_t len = (chunk->pkt.len - GORM_L2CAP_SDUHDR_LEN);
    memcpy(buf, (chunk->pkt.pdu + GORM_L2CAP_SDUHDR_LEN), len);
    buf += len;

    chunk = gorm_buf_deq(&coc->rxq);
    while (chunk) {
        len = (chunk->pkt.len - GORM_L2CAP_HDR_LEN);
        memcpy(buf, (chunk->pkt.pdu + GORM_L2CAP_HDR_LEN), len);
        buf += len;

        chunk = gorm_buf_deq(&coc->rxq);
    }
}

int gorm_coc_send(gorm_coc_t *coc, iolist_t *data)
{
    assert(coc);
    assert(data);

    size_t len = iolist_size(data);
    if (len > coc->mps_peer) {
        return GORM_ERR_OVERFLOW;
    }

    /* allocate enough buffers to fit the given data */
    gorm_bufq_t tmpq = GORM_BUFQ_INIT;
    size_t written = 0;
    size_t data_pos = 0;
    size_t mtu = (OWN_MTU < coc->mtu_peer) ? OWN_MTU : coc->mtu_peer;

    while (written < len) {
        gorm_buf_t *buf = gorm_buf_get();
        if (buf == NULL) {
            gorm_buf_return_q(&tmpq);
            return -ENOBUFS;
        }

        size_t length = 0;

        /* set SDU length on first chunk */
        if (written == 0) {
            gorm_util_htoles(&buf->pkt.pdu[4], len);
            length += 2;
        }

        do {
            size_t iol_rest = data->iol_len - data_pos;
            size_t capacity = mtu - length;
            size_t copy = (iol_rest < capacity) ? iol_rest : capacity;
            uint8_t *tmp = data->iol_base;
            memcpy(&buf->pkt.pdu[length + 4], &tmp[data_pos], copy);
            written += copy;
            length += copy;
            data_pos += copy;
            if (data_pos == data->iol_len) {
                data = data->iol_next;
                data_pos = 0;
            }
        } while (data && (length < mtu));

        /* we finished one chunk, set the l2cap header length and CID */
        gorm_util_htoles(&buf->pkt.pdu[0], length);
        gorm_util_htoles(&buf->pkt.pdu[2], coc->cid_peer);
    }

    /* after all is copied, hand the chunks to the link layer */
    gorm_buf_move_q(&coc->con->ll.txq, &tmpq);

    return 0;
}

int gorm_coc_send_flat(gorm_coc_t *coc, void *data, size_t len)
{
    iolist_t iol = {
        .iol_next = NULL,
        .iol_base = data,
        .iol_len = len,
    };
    return gorm_coc_send(coc, &iol);
}

static int _coc_open(gorm_ctx_t *con, gorm_coc_t *coc,
                    uint16_t psm, uint16_t mps,
                    gorm_coc_cb_t on_data_cb, void *arg,
                    int initiate)
{
    return _coc_open(con, coc, psm, mps, on_data_cb, arg, 1);
    assert(con);
    assert(coc);
    assert(on_data_cb);

    gorm_buf_t *buf;

    /* validate current state */
    if (coc->con != NULL) {
        return -EBUSY;
    }
    if ((psm == 0) || (mps < 23)) {     // TODO magic number...
        return -EINVAL;
    }
    if (_find_coc_by_psm(con, psm)) {
        return -EALREADY;
    }
    if (initiate) {
        buf = gorm_buf_get();
        if (buf == NULL) {
            return -ENOMEM;
        }
    }

    /* build COC state */
    memset(coc, 0, sizeof(gorm_coc_t));
    coc->con = con;
    coc->psm = psm;
    coc->cid_own = gorm_l2cap_cid_gen(con);
    coc->mps_own = mps;
    coc->on_data = on_data_cb;
    coc->arg = arg;
    /* attach COC to connection */
    gorm_l2cap_coc_attach(con, coc);

    /* are we initiating the connection? */
    if (initiate) {
        uint8_t *data = &buf->pkt.pdu[GORM_L2CAP_HDR_LEN];
        data[0] = BLE_L2CAP_SIG_CON_RESP;
        data[1] = gorm_l2cap_sig_next_identifier(con);
        gorm_util_htoles(&data[2], GORM_L2CAP_COC_CONN_REQ_PKT_LEN);
        gorm_util_htoles(&data[4], psm);
        gorm_util_htoles(&data[6], coc->cid_own);
        gorm_util_htoles(&data[8], OWN_MTU);
        gorm_util_htoles(&data[10], coc->mps_own);
        gorm_util_htoles(&data[12], (coc->mps_own + (OWN_MTU - 1)) / OWN_MTU);
        gorm_l2cap_send(con, buf, GORM_L2CAP_COC_CONN_REQ_PKT_LEN + 4,
                                  GORM_L2CAP_CID_LE_SIGNAL);
    }
}

/* called from application thread context! */
int gorm_coc_accept(gorm_ctx_t *con, gorm_coc_t *coc,
                    uint16_t psm, uint16_t mps,
                    gorm_coc_cb_t on_data_cb, void *arg)
{
    return _coc_open(con, coc, psm, mps, on_data_cb, arg, 0);
}

int gorm_coc_connect(gorm_ctx_t *con, gorm_coc_t *coc,
                     uint16_t psm, uint16_t mps,
                     gorm_coc_cb_t on_data_cb, void *arg)
{
    return _coc_open(con, coc, psm, mps, on_data_cb, arg, 1);
}

int gorm_coc_on_con_req(gorm_ctx_t *con, gorm_buf_t *buf,
                        uint8_t *data, size_t size)
{
    uint16_t len = gorm_util_letohs(&data[2]);
    // TODO: validate len!
    (void)size;
    (void)len;

    /* prepare generic part of response */
    data[0] = BLE_L2CAP_SIG_CON_RESP;
    gorm_util_htoles(&data[2], GORM_L2CAP_COC_CONN_RSP_PKT_LEN);
    memset(&data[4], 0, GORM_L2CAP_COC_CONN_RSP_PKT_LEN);

    /* see if we are waiting for an incoming connection */
    uint16_t spsm = gorm_util_letohs(&data[4]);
    gorm_coc_t *coc = _find_coc_by_psm(con, spsm);
    if (coc == NULL) {
        /* we do not expect a connection for that service, signal this to peer */
        gorm_util_htoles(&data[12], GORM_COC_RESP_REFUSED_RESSOURCE);
    }
    else if (coc->cid_peer != 0) {
        gorm_util_htoles(&data[12], GORM_COC_RESP_REFUSED_CID_INUSE);
    }
    else {
        /* connect success, set state */
        coc->cid_peer = gorm_util_letohs(&data[6]);
        coc->mtu_peer = gorm_util_letohs(&data[8]);
        coc->mps_peer = gorm_util_letohs(&data[10]);
        coc->credits = gorm_util_letohs(&data[12]);

        /* and confirm */
        gorm_util_htoles(&data[4], coc->cid_own);
        gorm_util_htoles(&data[6], OWN_MTU);
        gorm_util_htoles(&data[8], coc->mps_own);
        gorm_util_htoles(&data[10], (coc->mps_own + (OWN_MTU - 1)) / OWN_MTU);
        /* no need to set result field, as all zeros means success */
    }

    /* send reply */
    gorm_l2cap_reply(con, buf, 14);

    return 0;
}
