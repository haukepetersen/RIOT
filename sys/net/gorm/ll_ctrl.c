
#include <string.h>

#include "net/gorm/util.h"
#include "net/gorm/ll/chan.h"
#include "net/gorm/ll/ctrl.h"

#define ENABLE_DEBUG                (1)
#include "debug.h"

#define CON_UPDATE_LEN              (9U)

static const uint64_t features = (0UL
#ifdef GORM_LL_FEAT_LE_ENC
    | GORM_LL_FEAT_LE_ENC_FLAG
#endif
#ifdef GORM_LL_FEAT_PING
    | GORM_LL_FEAT_PING_FLAG
#endif
    /* TODO: add all 17 possible flags */
);


static void _connection_update(gorm_ll_ctx_t *con, gorm_buf_t *buf)
{
    con->timings_update.instant =
        (gorm_util_letohs(&buf->pkt.pdu[1 + CON_UPDATE_LEN]));
    memcpy(con->timings_update.raw, &buf->pkt.pdu[1], CON_UPDATE_LEN);
    gorm_buf_return(buf);
}

static void _channel_update(gorm_ll_ctx_t *con, gorm_buf_t *buf)
{
    con->chan_update.instant =
        (gorm_util_letohs(&buf->pkt.pdu[1 + BLE_CHANMAP_LEN]));
    memcpy(con->chan_update.map, &buf->pkt.pdu[1], BLE_CHANMAP_LEN);
    con->chan_update.cnt = gorm_ll_chan_count(con->chan_update.map);
    gorm_buf_return(buf);
}

static void _terminate(gorm_ll_ctx_t *con, gorm_buf_t *buf)
{
    gorm_buf_return(buf);
    gorm_ll_terminate(con);
}

static void _feature_resp(gorm_ctx_t *con, gorm_buf_t *buf)
{
    buf->pkt.len = sizeof(uint64_t) + 1;
    buf->pkt.pdu[0] = BLE_LL_FEATURE_RSP;
    memcpy(&buf->pkt.pdu[1], &features, sizeof(uint64_t));
    gorm_buf_enq(&con->ll.txq, buf);
}

static void _ctrl_version(gorm_ctx_t *con, gorm_buf_t *buf)
{
    buf->pkt.len = 6;
    buf->pkt.pdu[0] = BLE_LL_VERSION_IND;
    buf->pkt.pdu[1] = GORM_CFG_VERSION;
    gorm_util_htoles(&buf->pkt.pdu[2], GORM_CFG_VENDOR_ID);
    gorm_util_htoles(&buf->pkt.pdu[4], GORM_CFG_SUB_VERS_NR);
    gorm_buf_enq(&con->ll.txq, buf);
}

void gorm_ll_ctrl_on_data(gorm_ctx_t *con, gorm_buf_t *buf)
{
    switch (buf->pkt.pdu[0]) {
        case BLE_LL_CONN_UPDATE_IND:
            _connection_update(&con->ll, buf);
            DEBUG("[gorm_ll_ctrl] on_data: processed UPDATE_IND\n");
            break;
        case BLE_LL_CHANNEL_MAP_IND:
            _channel_update(&con->ll, buf);
            DEBUG("[gorm_ll_ctrl] on_data: processed CHANNEL_MAP_IND\n");
            break;
        case BLE_LL_TERMINATE_IND:
            _terminate(&con->ll, buf);
            DEBUG("[gorm_ll_ctrl] on_data: processed TERMINATE_IND\n");
            break;
        case BLE_LL_FEATURE_REQ:
            _feature_resp(con, buf);
            DEBUG("[gorm_ll_ctrl] on_data: responded to FEATURE_REQ\n");
            break;
        case BLE_LL_VERSION_IND:
            _ctrl_version(con, buf);
            DEBUG("[gorm_ll_ctrl] on_data: responded to VERSION_IND\n");
            break;
        case BLE_LL_ENC_REQ:
        case BLE_LL_ENC_RSP:
        case BLE_LL_START_ENC_REQ:
        case BLE_LL_START_ENC_RSP:
        case BLE_LL_UNKNOWN_RSP:
        case BLE_LL_FEATURE_RSP:
        case BLE_LL_PAUSE_ENC_REQ:
        case BLE_LL_PAUSE_ENC_RSP:
        case BLE_LL_REJECT_IND:
        case BLE_LL_SLAVE_FEATURE_REQ:
        case BLE_LL_CONN_PARAM_REQ:
        case BLE_LL_CONN_PARAM_RSP:
        case BLE_LL_REJECT_EXT_IND:
        case BLE_LL_PING_REQ:
        case BLE_LL_PING_RSP:
        case BLE_LL_LENGTH_REQ:
        case BLE_LL_LENGTH_RSP:
        case BLE_LL_PHY_REQ:
        case BLE_LL_PHY_RSP:
        case BLE_LL_PHY_UPDATE_IND:
        case BLE_LL_MIN_USED_CHAN_IND:
            DEBUG("[gorm_ll_ctrl] on_data: unknown opcode\n");
            gorm_buf_return(buf);
            break;
    }
}
