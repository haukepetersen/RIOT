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

#define ENABLE_DEBUG        (0)
#include "debug.h"

#define PARENT_NONE         (-1)
#define PARENT_ROOT         (-2)

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
#define CONN_NUMOF          (MYNEWT_VAL_BLE_MAX_CONNECTIONS)
#define PARENT_NUM          (NIMBLE_RPBLE_MAXPARENTS)
#define CHILD_NUM           (CONN_NUMOF - PARENT_NUM)


typedef struct {
    ble_addr_t addr;    /**< potential parents address */
    unsigned score;         /* 0 := not used, larger is better! */
    uint8_t tries;
} ppt_entry_t;

/* keep the timing parameters for connections and advertisements */
static struct ble_gap_adv_params _adv_params = { 0 };
static struct ble_gap_conn_params _conn_params = { 0 };
static uint32_t _conn_timeout;   /* in ms */

/* local RPL context */
static nimble_rpble_ctx_t _local_rpl_ctx;
static int _current_parent = PARENT_NONE;
/* table for keeping possible parents */
static ppt_entry_t _ppt[NIMBLE_RPBLE_PPTSIZE];

/* eval event used for periodical state updates */
static uint32_t _eval_itvl;
static struct ble_npl_callout _evt_eval;

static nimble_netif_eventcb_t _eventcb = NULL;


static void _parent_find(void);
static void _parent_select(struct ble_npl_event *ev);

static ppt_entry_t *_ppt_find_best(void)
{
    ppt_entry_t *best = &_ppt[0];

    for (unsigned i = 1; i < NIMBLE_RPBLE_PPTSIZE; i++) {
        if (_ppt[i].score > best->score) {
            best = &_ppt[i];
        }
    }

    return (best->score > 0) ? best : NULL;
}

static ppt_entry_t *_ppt_find_worst(void)
{
    ppt_entry_t *worst = &_ppt[0];

    for (unsigned i = 1; i < NIMBLE_RPBLE_PPTSIZE; i++) {
        if (worst->score == 0) {
            return worst;
        }
        if (_ppt[i].score < worst->score) {
            worst = &_ppt[i];
        }
    }

    return worst;
}

static void _ppt_clear(void)
{
    memset(_ppt, 0, sizeof(_ppt));
}

static void _ppt_add(const ble_addr_t *addr, unsigned score)
{
    /* if peer is already in table, update its entry */
    for (unsigned i = 0; i < NIMBLE_RPBLE_PPTSIZE; i++) {
        if (memcmp(addr, &_ppt[i].addr, sizeof(ble_addr_t)) == 0) {
            _ppt[i].score = score;
        }
    }

    /* if not, find the worst entry in the table and override it if feasible */
    ppt_entry_t *e  = _ppt_find_worst();
    if (score > e->score) {
        memcpy(&e->addr, addr, sizeof(ble_addr_t));
        e->score = score;
        e->tries = 0;
    }
}

static unsigned _ppt_score(uint8_t *buf, size_t len)
{
    uint16_t rank = ((int16_t)buf[POS_RANK + 1] << 8) | buf[POS_RANK];  // TODO: proper byteorder
    unsigned free = (unsigned)buf[POS_FREE_SLOTS];
    return (60000 - rank + free); // TODO: how large can RPL ranks become?
}

static void _make_ad(bluetil_ad_t *ad, uint8_t *buf,
                     const nimble_rpble_ctx_t *ctx, uint8_t free_slots)
{
    int res;

    /* fill manufacturer specific data field */
    uint8_t msd[VENDOR_FIELD_LEN];
    msd[0] = 0xff;  /* vendor ID part 1 */
    msd[1] = 0xff;  /* vendor ID part 2 */
    msd[2 + POS_INST_ID] = ctx->inst_id;
    memcpy(&msd[2 + POS_DODAG_ID], ctx->dodag_id, 16);
    msd[2 + POS_VERSION] = ctx->version;
    msd[2 + POS_RANK] = (ctx->rank & 0xff);     /* TODO: fix byteorder conversion */
    msd[2 + POS_RANK + 1] = (ctx->rank >> 8);
    msd[2 + POS_FREE_SLOTS] = free_slots;

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
    _make_ad(&ad, buf, &_local_rpl_ctx,
             (uint8_t)nimble_netif_conn_count(NIMBLE_NETIF_UNUSED) + 1);
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

    // TODO: filter peer for DODAG-ID, version, or similar?
    /* lets score the result */
    unsigned score = _ppt_score((msd_field.data + 2), (msd_field.len - 2));
    _ppt_add(addr, score);
}

static void _parent_find(void)
{
    _ppt_clear();
    nimble_scanner_start();
    ble_npl_callout_reset(&_evt_eval, _eval_itvl);
}

static void _parent_find_stop(void)
{
    nimble_scanner_stop();
    ble_npl_callout_stop(&_evt_eval);
    _ppt_clear();
}

static void _parent_select(struct ble_npl_event *ev)
{
    (void)ev;

    /* for now, we only try to connect to a parent if we have none */
    assert(_current_parent == PARENT_NONE);

    /* reset timer and stop scanner  */
    ble_npl_callout_stop(&_evt_eval);
    nimble_scanner_stop();

    /* just in case this event is triggered while we were configured to be the
     * RPL root */
    if (_local_rpl_ctx.role == GNRC_RPL_ROOT_NODE) {
        DEBUG("# eval: we are root, so no more parent evalutations\n");
        return;
    }

    ppt_entry_t *pp = _ppt_find_best();
    if (pp == NULL) {
        DEBUG("# parent_select: no fitting parent in PPT, continue search\n");
        _parent_find();
        return;
    }

    DEBUG("# parent selected. will connect now: %02x\n", (int)pp->addr.val[5]);
    pp->tries++;
    int res = nimble_netif_connect(&pp->addr, &_conn_params, _conn_timeout);
    if (res < 0) {
        DEBUG("# parent_handler: err: unable to connect to preferred parent\n");
        _parent_find();
    }
    else {
        _current_parent = res;
    }
}

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

static void _on_netif_evt(int handle, nimble_netif_event_t event,
                          const uint8_t *addr)
{
    (void)addr;

    switch (event) {
        case NIMBLE_NETIF_CONNECTED_MASTER:
            assert(_current_parent == handle);
            _dbg_msg("PARENT selected", addr);
            _children_accept();
            break;
        case NIMBLE_NETIF_CONNECTED_SLAVE:
            _dbg_msg("CHILD added", addr);
            _children_accept();
            break;
        case NIMBLE_NETIF_CLOSED_MASTER:
            _dbg_msg("PARENT lost", addr);
            _current_parent = PARENT_NONE;
            nimble_netif_accept_stop();
            /* back to 0, now we need to find a new parent... */
            _parent_find();
            break;
        case NIMBLE_NETIF_CLOSED_SLAVE:
            _dbg_msg("CHILD lost", addr);
            _children_accept();
            break;
        case NIMBLE_NETIF_ABORT_MASTER:
            _dbg_msg("PARENT abort", addr);
            _current_parent = PARENT_NONE;
            _parent_find();
            break;
        case NIMBLE_NETIF_ABORT_SLAVE:
            _dbg_msg("CHILD abort", addr);
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


    /** initialize the eval event */
    ble_npl_time_ms_to_ticks(cfg->eval_itvl / 1000, &_eval_itvl);
    ble_npl_callout_init(&_evt_eval, nimble_port_get_dflt_eventq(),
                         _parent_select, NULL);

    _adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    _adv_params.disc_mode = BLE_GAP_DISC_MODE_LTD;
    _adv_params.itvl_min = (cfg->adv_itvl / BLE_HCI_ADV_ITVL);
    _adv_params.itvl_max = (cfg->adv_itvl / BLE_HCI_ADV_ITVL);

    _conn_params.scan_itvl = (cfg->conn_scanitvl / BLE_HCI_SCAN_ITVL);
    _conn_params.scan_window = (cfg->conn_scanitvl / BLE_HCI_SCAN_ITVL);
    _conn_params.itvl_min = (cfg->conn_itvl / BLE_HCI_CONN_ITVL);
    _conn_params.itvl_max = (cfg->conn_itvl / BLE_HCI_CONN_ITVL);
    _conn_params.latency = cfg->conn_latency;
    _conn_params.supervision_timeout = (cfg->conn_super_to / 1000 /
                                        BLE_HCI_CONN_SPVN_TMO_UNITS);
    _conn_timeout = cfg->conn_timeout / 1000;

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

    DEBUG("[rpble] RPL update\n");

    /* XXX: if the update context is equal to what we have, ignore it */
    _local_rpl_ctx.free_slots = 0;
    if (memcmp(&_local_rpl_ctx, ctx, sizeof(nimble_rpble_ctx_t)) == 0) {
        DEBUG("[rpble] RPL update: ignored (no ctx data change)\n");
        return NIMBLE_RPBLE_NO_CHANGE;
    }

    DEBUG("[rpble] RPL update: got new context data (rank %i, inst %i)\n",
          (int)ctx->rank, (int)ctx->inst_id);

    /* save rpl context for future reference */
    memcpy(&_local_rpl_ctx, ctx, sizeof(nimble_rpble_ctx_t));

    if (ctx->role == GNRC_RPL_ROOT_NODE) {
        _parent_find_stop();
        _current_parent = PARENT_ROOT;
    }

    /* advertise the updated context */
    _children_accept();

    return NIMBLE_RPBLE_OK;
}
