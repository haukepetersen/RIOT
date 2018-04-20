/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm_l2cap
 * @{
 *
 * @file
 * @brief       Gorm's L2CAP layer implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "net/gorm/util.h"
#include "net/gorm/l2cap.h"
#include "net/gorm/gatt.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

void gorm_l2cap_reply(gorm_ctx_t *con, gorm_buf_t *buf, uint16_t data_len)
{
    gorm_util_htoles(buf->pkt.pdu, data_len);
    buf->pkt.len = (uint8_t)(data_len + GORM_L2CAP_HDR_LEN);
    gorm_buf_enq(&con->ll.txq, buf);
}

void gorm_l2cap_on_data(gorm_ctx_t *con, uint8_t llid, gorm_buf_t *buf)
{
    (void)llid; /* for future use */

    size_t len = (size_t)gorm_util_letohs(&buf->pkt.pdu[0]);
    uint16_t cid = gorm_util_letohs(&buf->pkt.pdu[2]);
    uint8_t *data = &buf->pkt.pdu[4];

    switch (cid) {
        case GORM_L2CAP_CID_ATT:
            gorm_gatt_on_data(con, buf, data, len);
            break;
        case GORM_L2CAP_CID_LE_SIGNAL:
        case GORM_L2CAP_CID_SM:
        default:
            DEBUG("[gorm_l2cap] on_data: CID (%i) not supported (yet)\n",
                  (int)cid);
            gorm_buf_return(buf);
            break;
    }
}
