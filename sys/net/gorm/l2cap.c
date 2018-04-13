


#include "net/gorm/util.h"
#include "net/gorm/l2cap.h"
#include "net/gorm/gatt.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"



int gorm_l2cap_send(gorm_ctx_t *con, void *data, size_t len)
{
    (void)con;
    (void)data;

    /* TODO: split data into packets and pass those to the link layer */
    // uint8_t *buf = (uint8_t *)data;
    // size_t needed = (len + 2);
    // /* TODO: use different means to find out the max PDU len */
    // size_t num_pdus = (needed / NETDEV_BLE_PDU_MAXLEN);
    // gorm_buf_t *head = gorm_pdupool_get(num_pdus);
    // if (head == NULL) {
    //     DEBUG("[gorm_l2cap] send: unable to allocate necessary buffers\n");
    //     return -1;
    // }
    // size_t pos = 2;
    /* TODO: fill l2cap data header into first packet */
    /* TODO: copy data into PDU buffers */
    return len;
}

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

    /* TODO: handle fragmentation / re-assembly */

    switch (cid) {
        case GORM_L2CAP_CID_ATT:
            DEBUG("[gorm_l2cap] on_data: got ATT data\n");
            /* TODO: handle data */
            gorm_gatt_on_data(con, buf, data, len);
            break;
        case GORM_L2CAP_CID_LE_SIGNAL:
        case GORM_L2CAP_CID_SM:
        default:
            DEBUG("[gorm_l2cap] on_data: CID (%i) not supported\n", (int)cid);
            /* TODO: implement */
            gorm_buf_return(buf);
            break;
    }
}
