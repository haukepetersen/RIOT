/*
 * Copyright (C) 2019-2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     nimble_rpble
 * @{
 *
 * @file
 * @brief       RPL-over-BLE (rpble) implementation for NimBLE and GNRC
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Cenk Gündoğan <cenk.guendogan@haw-hamburg.de>
 *
 * @}
 */

#include <limits.h>

#include "mutex.h"
#include "thread.h"
#include "random.h"
#include "net/bluetil/ad.h"
#include "net/bluetil/addr.h"
#include "net/gnrc/rpl.h"

#include "nimble_netif.h"
#include "nimble_netif_conn.h"
#include "nimble_rpble.h"
#include "nimble_scanner.h"
#include "host/ble_gap.h"
#include "nimble/nimble_port.h"

#ifdef MODULE_EXPSTATS
#include "expstats.h"
#define EXPSTAT(x)      expstats_log(x)
#else
#define EXPSTAT(x)
#endif

#define ENABLE_DEBUG        (0)
#include "debug.h"

#define PARENT_NONE         (-1)
#define PARENT_ROOT         (-2)
#define SCORE_NONE          (0U)

#define VENDOR_FIELD_LEN    (23U)

/* PPT fields */
#define PPT_STATE_FREE      (0U)
#define PPT_STATE_FILLED    (1U)

/* RPL meta data context fields */
#define POS_INST_ID         (0)
#define POS_DODAG_ID        (1)
#define POS_VERSION         (17)
#define POS_RANK            (18)
#define POS_FREE_SLOTS      (20)

/* get shortcuts for the max number of connections, parents, and children */
#define PARENT_NUM          (1U)
#define CONN_NUMOF          (MYNEWT_VAL_BLE_MAX_CONNECTIONS)
#define CHILD_NUM           (CONN_NUMOF - PARENT_NUM)



/* keep the timing parameters for connections and advertisements */
static struct ble_gap_adv_params _adv_params = { 0 };
static struct ble_gap_conn_params _conn_params = { 0 };
static uint32_t _conn_timeout;      /* in ms */
static uint32_t _conn_itvl_min;     /* in ms */
static uint32_t _conn_itvl_max;     /* in ms */

/* local RPL context */
static nimble_rpble_ctx_t _local_rpl_ctx;
static int _current_parent = PARENT_NONE;
struct {
    ble_addr_t addr;        /**< address of highest scored potential parent */
    uint16_t score;         /* 0 := not used, larger is better! */
} _psel = { { 0 }, SCORE_NONE };

/* eval event used for periodical state updates */
static uint32_t _eval_itvl;
static struct ble_npl_callout _evt_eval;

static nimble_netif_eventcb_t _eventcb = NULL;


#if ENABLE_DEBUG
static void _dbg_msg(const char *text, const uint8_t *addr)
{
    printf("[rpble] %s (", text);
    bluetil_addr_print(addr);
    printf(")\n");
}
#else
static void _dbg_msg(const char *text, const uint8_t *addr)
{
    (void)text;
    (void)addr;
}
#endif

static uint16_t _psel_score(uint16_t rank, uint8_t free)
{
    return (UINT16_MAX - rank + free);
}

static void _make_ad(bluetil_ad_t *ad, uint8_t *buf)
{
    int res;

    /* fill manufacturer specific data field */
    uint8_t msd[VENDOR_FIELD_LEN];
    msd[0] = 0xff;  /* vendor ID part 1 */
    msd[1] = 0xff;  /* vendor ID part 2 */
    msd[2 + POS_INST_ID] = _local_rpl_ctx.inst_id;
    memcpy(&msd[2 + POS_DODAG_ID], _local_rpl_ctx.dodag_id, 16);
    msd[2 + POS_VERSION] = _local_rpl_ctx.version;
    byteorder_htobebufs(&msd[2 + POS_RANK], _local_rpl_ctx.rank);
    msd[2 + POS_FREE_SLOTS] = (uint8_t)nimble_netif_conn_count(NIMBLE_NETIF_UNUSED);

    res = bluetil_ad_init_with_flags(ad, buf, BLE_HS_ADV_MAX_SZ,
                                     BLUETIL_AD_FLAGS_DEFAULT);
    assert(res == BLUETIL_AD_OK);
    res = bluetil_ad_add(ad, BLE_GAP_AD_VENDOR, msd, sizeof(msd));
    assert(res == BLUETIL_AD_OK);
    (void)res;
}

static void _children_accept(void)
{
    /* stop advertising for updating */
    nimble_netif_accept_stop();

    /* we only start advertising (accepting) if we do have an active parent and
     * if we have resources for new connections */
    if ((_current_parent == PARENT_NONE) ||
        (nimble_netif_conn_count(NIMBLE_NETIF_UNUSED) == 0)) {
        return;
    }

    /* generate the new advertisement data */
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    _make_ad(&ad, buf);
    /* start advertising this node */
    int res = nimble_netif_accept(ad.buf, ad.pos, &_adv_params);
    if (res != NIMBLE_NETIF_OK) {
        DEBUG("[rpble] children_accept: fail to start accepting: %i\n", res);
    }
    assert(res == NIMBLE_NETIF_OK);
    (void)res;
}

static void _on_scan_evt(uint8_t type,
                         const ble_addr_t *addr, int8_t rssi,
                         const uint8_t *ad, size_t ad_len)
{
    int res;

    /* filter out all non-connectible advertisements */
    if (type != BLE_HCI_ADV_TYPE_ADV_IND) {
        return;
    }

    /* check if scanned node does actually speak rpble */
    bluetil_ad_data_t msd_field;
    bluetil_ad_t ads = { .buf = (uint8_t *)ad, .pos = ad_len, .size = ad_len };
    res = bluetil_ad_find(&ads, BLE_GAP_AD_VENDOR, &msd_field);
    if (res != BLUETIL_AD_OK) {
        return;
    }
    if (msd_field.len != VENDOR_FIELD_LEN ||
        msd_field.data[0] != 0xff || msd_field.data[1] != 0xff) {
        return;
    }

    /**
     * @todo    Here we need to improve the filtering: so far, we consider every
     *          node we see that is capable of rplbe to be a parent. We should
     *          however also filter for instance ID and possibly the DODAG ID as
     *          well. On top, we should probably only consider nodes with >=
     *          version as parent
     */

    /* score and compare advertising peer */
    uint16_t rank = byteorder_bebuftohs(&msd_field.data[2 + POS_RANK]);
    uint8_t free = msd_field.data[2 + POS_FREE_SLOTS];
    uint16_t score = _psel_score(rank, free);

    /* our currently preferred parent might have updated its score in the mean
     * time, so we need to check that */
    if (memcmp(&_psel.addr, addr, sizeof(ble_addr_t)) == 0) {
        _psel.score = score;
        return;
    }

    /* we consider only parents with a lower rank and remember the one with the
     * best score */
    if (((_local_rpl_ctx.rank == 0) || (_local_rpl_ctx.rank > rank)) &&
        (score > _psel.score)) {
        _psel.score = score;
        memcpy(&_psel.addr, addr, sizeof(ble_addr_t));
    }
}

static void _parent_find(void)
{
    _psel.score = SCORE_NONE;
    nimble_scanner_start();
    ble_npl_callout_reset(&_evt_eval, _eval_itvl);
}

static void _parent_find_stop(void)
{
    ble_npl_callout_stop(&_evt_eval);
    nimble_scanner_stop();
}

static void _parent_connect(struct ble_npl_event *ev)
{
    (void)ev;

    // TODO: we probably need to make this thread safe... */
    /* for now, we only try to connect to a parent if we have none */
    assert(_current_parent == PARENT_NONE);
    /* just in case this event is triggered while we were configured to be the
     * RPL root */
    if (_local_rpl_ctx.role == GNRC_RPL_ROOT_NODE) {
        DEBUG("[rpble] we are root, so no more parent evalutations\n");
        return;
    }

    /* reset timer and stop scanner  */
    _parent_find_stop();

    if (_psel.score == SCORE_NONE) {
        /* no potential parent found, restarting search for one */
        DEBUG("[rpble] no parent found, restarting search\n");
        _parent_find();
        return;
    }

    _dbg_msg("parent found, trying to connect", _psel.addr.val);

    /* set a random connection itvl */
    uint32_t itvl = random_uint32_range(_conn_itvl_min, _conn_itvl_max);
    _conn_params.itvl_min = (itvl / BLE_HCI_CONN_ITVL);
    _conn_params.itvl_max = _conn_params.itvl_min;
    int res = nimble_netif_connect(&_psel.addr, &_conn_params, _conn_timeout);
    if (res < 0) {
        DEBUG("# err: unable to start connection to parent\n");
        _parent_find();
        return;
    }
    _current_parent = res;
}

static void _on_netif_evt(int handle, nimble_netif_event_t event,
                          const uint8_t *addr)
{
    (void)addr;

    switch (event) {
        case NIMBLE_NETIF_CONNECTED_MASTER:
            assert(_current_parent == handle);
            _dbg_msg("PARENT selected", addr);
            EXPSTAT(EXPSTATS_RPBLE_PARENT_CONN);
            /* send a DIS once connected to a (new) parent) */
            gnrc_rpl_send_DIS(NULL, (ipv6_addr_t *) &ipv6_addr_all_rpl_nodes, NULL, 0);
            _children_accept();
            break;
        case NIMBLE_NETIF_CONNECTED_SLAVE:
            _dbg_msg("CHILD added", addr);
            EXPSTAT(EXPSTATS_RPBLE_CHILD_CONN);
            _children_accept();
            break;
        case NIMBLE_NETIF_CLOSED_MASTER:
            nimble_netif_accept_stop();
            _dbg_msg("PARENT lost", addr);
            EXPSTAT(EXPSTATS_RPBLE_PARENT_LOST);
            _current_parent = PARENT_NONE;
            /* back to 0, now we need to find a new parent... */
            _parent_find();
            break;
        case NIMBLE_NETIF_CLOSED_SLAVE:
            _dbg_msg("CHILD lost", addr);
            EXPSTAT(EXPSTATS_RPBLE_CHILD_LOST);
            _children_accept();
            break;
        case NIMBLE_NETIF_ABORT_MASTER:
            nimble_netif_accept_stop();
            _dbg_msg("PARENT abort", addr);
            EXPSTAT(EXPSTATS_RPBLE_PARENT_ABORT);
            _current_parent = PARENT_NONE;
            _parent_find();
            break;
        case NIMBLE_NETIF_ABORT_SLAVE:
            _dbg_msg("CHILD abort", addr);
            EXPSTAT(EXPSTATS_RPBLE_CHILD_ABORT);
            _children_accept();
            break;
        case NIMBLE_NETIF_CONN_UPDATED:
            _dbg_msg("param update", addr);
            break;
        default:
            /* nothing to do for all other events */
            break;
    }

    /* pass events to high-level user if activated */
    if (_eventcb) {
        _eventcb(handle, event, addr);
    }
}

int nimble_rpble_init(const nimble_rpble_cfg_t *cfg)
{
    assert(cfg);

    memset(&_local_rpl_ctx, 0, sizeof(_local_rpl_ctx));

    /** initialize the eval event */
    uint32_t itvl = random_uint32_range(cfg->eval_itvl, (2 * cfg->eval_itvl));
    ble_npl_time_ms_to_ticks(itvl / 1000, &_eval_itvl);
    ble_npl_callout_init(&_evt_eval, nimble_port_get_dflt_eventq(),
                         _parent_connect, NULL);

    _adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    _adv_params.disc_mode = BLE_GAP_DISC_MODE_LTD;
    _adv_params.itvl_min = (cfg->adv_itvl / BLE_HCI_ADV_ITVL);
    _adv_params.itvl_max = (cfg->adv_itvl / BLE_HCI_ADV_ITVL);

    _conn_params.scan_itvl = (cfg->conn_scanitvl / BLE_HCI_SCAN_ITVL);
    _conn_params.scan_window = (cfg->conn_scanitvl / BLE_HCI_SCAN_ITVL);
    _conn_params.latency = cfg->conn_latency;
    _conn_params.supervision_timeout = (cfg->conn_super_to / 1000 /
                                        BLE_HCI_CONN_SPVN_TMO_UNITS);
    _conn_timeout = cfg->conn_timeout / 1000;
    _conn_itvl_min = cfg->conn_itvl_min;
    _conn_itvl_max = cfg->conn_itvl_max;

    /* register event callback */
    nimble_netif_eventcb(_on_netif_evt);

    /* configure scanner */
    struct ble_gap_disc_params scan_params = { 0 };
    scan_params.itvl = (cfg->scan_itvl / BLE_HCI_SCAN_ITVL);
    scan_params.window = (cfg->scan_win / BLE_HCI_SCAN_ITVL);
    scan_params.passive = 1;
    scan_params.filter_duplicates = 1;
    nimble_scanner_init(&scan_params, _on_scan_evt);

    DEBUG("[rpble] initialization done, starting to look for parents\n");

    /* start to look for parents */
    _parent_find();

    return NIMBLE_RPBLE_OK;
}

int nimble_rpble_eventcb(nimble_netif_eventcb_t cb)
{
    _eventcb = cb;
    return NIMBLE_RPBLE_OK;
}

int nimble_rpble_update(const nimble_rpble_ctx_t *ctx)
{
    assert(ctx != NULL);

    // DEBUG("[rpble] RPL update [inst_id:%i, rank:%i]\n",
    //       (int)ctx->inst_id, (int)ctx->rank);

    /* XXX: if the update context is equal to what we have, ignore it */
    _local_rpl_ctx.free_slots = 0;
    if (memcmp(&_local_rpl_ctx, ctx, sizeof(nimble_rpble_ctx_t)) == 0) {
        DEBUG("[rpble] RPL update: ignored (no ctx data change)\n");
        EXPSTAT(EXPSTATS_RPBLE_UPDATE_NO_CHANGE);
        return NIMBLE_RPBLE_NO_CHANGE;
    }

    DEBUG("[rpble] RPL update: got new context data (rank %i, inst %i)\n",
          (int)ctx->rank, (int)ctx->inst_id);

    /* save rpl context for future reference */
    memcpy(&_local_rpl_ctx, ctx, sizeof(nimble_rpble_ctx_t));
    EXPSTAT(EXPSTATS_RPBLE_UPDATE_NEW_CTX);

    if (ctx->role == GNRC_RPL_ROOT_NODE) {
        DEBUG("[rpble] RPL update: we are root, advertising now\n");
        _parent_find_stop();
        _current_parent = PARENT_ROOT;
    }

    /* advertise the updated context */
    _children_accept();

    return NIMBLE_RPBLE_OK;
}
