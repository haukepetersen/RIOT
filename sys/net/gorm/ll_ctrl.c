
#include <string.h>

#include "net/gorm/util.h"
#include "net/gorm/ll/chan.h"
#include "net/gorm/ll/ctrl.h"

#define ENABLE_DEBUG                1
#include "debug.h"

#define CON_UPDATE_LEN              (9U)

static const uint64_t features = (0UL
#ifdef MODULE_GORM_FEATURE_LL_ENC
    | BLE_LL_FEAT_ENC
#endif
#ifdef MODULE_GORM_FEATURE_LL_CONN_PARAM_REQ_PROCEDURE
    | BLE_LL_FEAT_CONN_PARAM_REQ_PROCEDURE
#endif
#ifdef MODULE_GORM_FEATURE_LL_EXT_REJECT_IND
    | BLE_LL_FEAT_EXT_REJECT_IND
#endif
#ifdef MODULE_GORM_FEATURE_LL_SLAVE_FEAT_EXCHANGE
    | BLE_LL_FEAT_SLAVE_FEAT_EXCHANGE
#endif
#ifdef MODULE_GORM_FEATURE_LL_PING
    | BLE_LL_FEAT_PING
#endif
#ifdef MODULE_GORM_FEATURE_LL_PACKET_LEN_EXT
    | BLE_LL_FEAT_PACKET_LEN_EXT
#endif
#ifdef MODULE_GORM_FEATURE_LL_PRIVACY
    | BLE_LL_FEAT_PRIVACY
#endif
#ifdef MODULE_GORM_FEATURE_LL_EXT_SCANNER_FILTER_POL
    | BLE_LL_FEAT_EXT_SCANNER_FILTER_POL
#endif
#ifdef MODULE_GORM_FEATURE_LL_2M_PHY
    | BLE_LL_FEAT_2M_PHY
#endif
#ifdef MODULE_GORM_FEATURE_LL_STABLE_MOD_INDEX_TX
    | BLE_LL_FEAT_STABLE_MOD_INDEX_TX
#endif
#ifdef MODULE_GORM_FEATURE_LL_STABLE_MOD_INDEX_RX
    | BLE_LL_FEAT_STABLE_MOD_INDEX_RX
#endif
#ifdef MODULE_GORM_FEATURE_LL_CODED_PHY
    | BLE_LL_FEAT_CODED_PHY
#endif
#ifdef MODULE_GORM_FEATURE_LL_EXTENDED_ADV
    | BLE_LL_FEAT_EXTENDED_ADV
#endif
#ifdef MODULE_GORM_FEATURE_LL_PERIODIC_ADV
    | BLE_LL_FEAT_PERIODIC_ADV
#endif
#ifdef MODULE_GORM_FEATURE_LL_CHAN_SEL_ALGO_2
    | BLE_LL_FEAT_CHAN_SEL_ALGO_2
#endif
#ifdef MODULE_GORM_FEATURE_LL_POWER_CLASS_1
    | BLE_LL_FEAT_POWER_CLASS_1
#endif
#ifdef MODULE_GORM_FEATURE_LL_MIN_NUM_USED_CHAN_PROCEDURE
    | BLE_LL_FEAT_MIN_NUM_USED_CHAN_PROCEDURE
#endif
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

#ifdef MODULE_GORM_FEATURE_LL_PING
static void _ping_resp(gorm_ctx_t *con, gorm_buf_t *buf)
{
    buf->pkt.len = 1;
    buf->pkt.pdu[0] = BLE_LL_PING_RSP;
    gorm_buf_enq(&con->ll.txq, buf);
}
#endif

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
        case BLE_LL_PING_REQ:
#ifdef MODULE_GORM_FEATURE_LL_PING
            _ping_resp(con, buf);
            DEBUG("[gorm_ll_ctrl] on_data: responded to PING_REQ\n");
            break;
#endif
        case BLE_LL_ENC_REQ:
        case BLE_LL_START_ENC_REQ:
        case BLE_LL_PAUSE_ENC_REQ:
        case BLE_LL_SLAVE_FEATURE_REQ:
        case BLE_LL_CONN_PARAM_REQ:
            buf->pkt.len = 2;
            buf->pkt.pdu[1] = buf->pkt.pdu[0];
            buf->pkt.pdu[0] = BLE_LL_UNKNOWN_RSP;
            gorm_buf_enq(&con->ll.txq, buf);
            break;

        case BLE_LL_ENC_RSP:
        case BLE_LL_START_ENC_RSP:
        case BLE_LL_UNKNOWN_RSP:
        case BLE_LL_FEATURE_RSP:
        case BLE_LL_PAUSE_ENC_RSP:
        case BLE_LL_REJECT_IND:
        case BLE_LL_CONN_PARAM_RSP:
        case BLE_LL_REJECT_EXT_IND:
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
