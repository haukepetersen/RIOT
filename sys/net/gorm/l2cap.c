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

#include "net/gorm.h"
#include "net/gorm/coc.h"
#include "net/gorm/gatt.h"
#include "net/gorm/util.h"
#include "net/gorm/l2cap.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

static void _on_signal_data(gorm_ctx_t *con, gorm_buf_t *buf,
                            uint8_t *data, size_t len)
{
    (void)con;
    (void)buf;

    /* if we do not at least have code, identifier, and length, we ignore the
     * packet */
    if (len < 4) {
        /* drop the packet */
        return;
    }

    uint8_t code = data[0];
    uint16_t sig_len = gorm_util_letohs(&data[2]);
    (void)sig_len;

    switch (code) {
        case BLE_L2CAP_SIG_DISCON_REQ:
        case BLE_L2CAP_SIG_CPU_REQ:
        case BLE_L2CAP_SIG_CON_REQ:
            DEBUG("[gorm_l2cap] on_signal: got unsupported req (%i)\n", (int)code);
            data[0] = BLE_L2CAP_SIG_REJECT;
            gorm_util_htoles(&data[2], 2);
            gorm_util_htoles(&data[4], BLE_L2CAP_SIG_REJ_NOT_UNDERSTOOD);
            gorm_l2cap_reply(con, buf, 6);
            break;
        case BLE_L2CAP_SIG_REJECT:
        case BLE_L2CAP_SIG_DISCON_RESP:
        case BLE_L2CAP_SIG_CPU_RESP:
        case BLE_L2CAP_SIG_CON_RESP:
        case BLE_L2CAP_SIG_FLOW_CREDIT:
        default:
            DEBUG("[gorm_l2cap] on_signal: unkown code (%i)\n", (int)code);
            break;
    }
}

void gorm_l2cap_reply(gorm_ctx_t *con, gorm_buf_t *buf, uint16_t data_len)
{
    gorm_util_htoles(buf->pkt.pdu, data_len);
    buf->pkt.len = (uint8_t)(data_len + GORM_L2CAP_HDR_LEN);
    gorm_buf_enq(&con->ll.txq, buf);
}

void gorm_l2cap_on_data(gorm_ctx_t *con, uint8_t llid, gorm_buf_t *buf)
{
    size_t len = (size_t)gorm_util_letohs(&buf->pkt.pdu[0]);
    uint16_t cid = gorm_util_letohs(&buf->pkt.pdu[2]);
    uint8_t *data = &buf->pkt.pdu[4];

    switch (cid) {
        case GORM_L2CAP_CID_ATT:
            gorm_gatt_on_data(con, buf, data, len);
            break;
        case GORM_L2CAP_CID_LE_SIGNAL:
            _on_signal_data(con, buf, data, len);
            break;
        case GORM_L2CAP_CID_SM:
            DEBUG("[gorm_l2cap] on_data: CID_SM not handled\n");
            gorm_buf_return(buf);
            break;
        default:
            gorm_coc_on_data(con, buf, cid, llid, data, len);
            break;
    }
}
