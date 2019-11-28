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

#include "net/gorm.h"
#include "net/gorm/util.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

static void _notify(gorm_coc_t *coc, unsigned event, void *val)
{
    /* TODO */
    (void)coc;
    (void)event;
    (void)val;
}

static gorm_coc_t *_find_coc(gorm_ctx_t *con, uint16_t cid)
{
    gorm_coc_t *tmp = con->l2cap.cocs;
    while (tmp != NULL) {
        if (tmp->cid_src == cid) {
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

void gorm_coc_read_to_flat(gorm_coc_t *coc, uint8_t *buf)
{
    assert(coc && buf);

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
    size_t len = iolist_size(data);
    if (len > coc->peer_mtu) {
        return GORM_ERR_OVERFLOW;
    }

    /* allocate enough buffers to fit the given data */
    gorm_bufq_t tmpq = GORM_BUFQ_INIT;
    unsigned num = ((len + 2) / (coc->peer_mps));

    for (unsigned i = 0; i < num; i++) {
        gorm_buf_t *buf = gorm_buf_get();
        if (buf == NULL) {
            gorm_buf_return_q(&tmp);
            return GORM_ERR_NOBUF;
        }
        gorm_buf_enq(&tmp, buf);
    }

    /* copy data into buffers */
    gorm_buf_t *buf = gorm_buf_deq(&tmp);
    _set_l2cap_hdr... TBD

        size_t pos = 0;

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
