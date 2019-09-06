/*
 * Copyright (C) 2019 Freie Universität Berlin
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
 * @brief       RPL over BLE implementation for NimBLE
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
#include "event/timeout.h"
#include "net/eui48.h"
#include "net/bluetil/ad.h"
#include "net/bluetil/addr.h"
#include "net/gnrc/rpl.h"
#include "event/timeout.h"

#include "nimble_netif.h"
#include "nimble_netif_conn.h"
#include "nimble_rpble.h"
#include "nimble_scanner.h"
#include "host/ble_gap.h"
#include "nimble/nimble_port.h"

#define PARENT_NONE         (-1)
#define PARENT_ROOT         (-2)

#define ENABLE_DEBUG        (1)
#include "debug.h"

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
    nimble_netif_rpble_ctx_t ctx;      /* TODO: only save rank and free slots? */
    ble_addr_t addr;    /**< potential parents address */
    uint8_t state;      /**< TODO: remove and use existing field implicitly */
    uint8_t tries;
} ppt_entry_t;

/* keep the timing parameters for connections and advertisements */
static struct ble_gap_adv_params _adv_params = { 0 };
static struct ble_gap_conn_params _conn_params = { 0 };
static uint32_t _conn_timeout;   /* in ms */

/* local RPL context */
static nimble_netif_rpble_ctx_t _local_rpl_ctx;
static int _current_parent = PARENT_NONE;
/* table for keeping possible parents */
static ppt_entry_t _ppt[NIMBLE_RPBLE_PPTSIZE];

/* eval event used for periodical state updates */
static uint32_t _eval_itvl;
static struct ble_npl_callout _evt_eval;

static void _parent_find(void);
static void _parent_select(struct ble_npl_event *ev);

static ppt_entry_t *_ppt_find_best(void)
{
    // DEBUG("# find best\n");
    uint16_t rank = UINT16_MAX;
    uint8_t free = 0;
    ppt_entry_t *pp = NULL;
    for (unsigned i = 0; i < NIMBLE_RPBLE_PPTSIZE; i++) {
        // DEBUG("# rank have %i, cmp %i, free have %i, cmp %i\n",
        //       (int)rank, (int)_ppt[i].ctx.rank,
        //       (int)free, (int)_ppt[i].ctx.free_slots);
        if ((_ppt[i].state == PPT_STATE_FILLED) &&
            ((_ppt[i].ctx.rank < rank) || (
             (_ppt[i].ctx.rank == rank) && (_ppt[i].ctx.free_slots < free)))) {
            rank = _ppt[i].ctx.rank;
            free = _ppt[i].ctx.free_slots;
            pp = &_ppt[i];
        }
    }
    return pp;
}

static ppt_entry_t *_ppt_find_worst(void)
{
    uint16_t rank = 0;
    uint8_t free = UINT8_MAX;
    ppt_entry_t *pp = NULL;
    for (unsigned i = 0; i < NIMBLE_RPBLE_PPTSIZE; i++) {
        if ((_ppt[i].state == PPT_STATE_FILLED) &&
            ((_ppt[i].ctx.rank > rank) || (
             (_ppt[i].ctx.rank == rank) && (_ppt[i].ctx.free_slots < free)))) {
            rank = _ppt[i].ctx.rank;
            free = _ppt[i].ctx.free_slots;
            pp = &_ppt[i];
        }
    }
    return pp;
}

static void _ppt_clear(void)
{
    memset(_ppt, 0, sizeof(_ppt));
}

static void _ppt_remove(const uint8_t *addr)
{
    for (unsigned i = 0; i < NIMBLE_RPBLE_PPTSIZE; i++) {
        if (memcmp(_ppt[i].addr.val, addr, BLE_ADDR_LEN) == 0) {
            _ppt[i].state = PPT_STATE_FREE;
            // DEBUG("# PPT remove: removed entry\n");
            return;
        }
    }
}

static void _ppt_add(nimble_netif_rpble_ctx_t *ctx, const ble_addr_t *addr)
{
    /* only add if the rank is interesting for us */
    if ((_local_rpl_ctx.rank != 0) && (ctx->rank > _local_rpl_ctx.rank)) {
        // DEBUG("# PPT add: new rank is larger (%i) then ours (%i)\n",
        //       (int)ctx->rank, (int)_local_rpl_ctx.rank);
        return;
    }

    // DEBUG("# PPT add %02x\n", (int)addr->val[5]);
    // for (unsigned i = 0; i < NIMBLE_RPBLE_PPTSIZE; i++) {
    //     DEBUG("# ppt[%u]: %02x rank: %i, free_slots: %i\n", i,
    //           (int)_ppt[i].addr.val[5],
    //           (int)_ppt[i].ctx.rank,
    //           (int)_ppt[i].ctx.free_slots);
    // }

    /* scan table if given address is already part of it. If so, update entry */
    for (unsigned i = 0; i < NIMBLE_RPBLE_PPTSIZE; i++) {
        if (memcmp(addr, &_ppt[i].addr, sizeof(ble_addr_t)) == 0) {
            // DEBUG("# PPT match\n");
            memcpy(&_ppt[i].ctx, ctx, sizeof(nimble_netif_rpble_ctx_t));
            return;
        }
    }
    /* if not found, look for an empty slot */
    for (unsigned i = 0; i < NIMBLE_RPBLE_PPTSIZE; i++) {
        if (_ppt[i].state == PPT_STATE_FREE) {
            memcpy(&_ppt[i].ctx, ctx, sizeof(nimble_netif_rpble_ctx_t));
            memcpy(&_ppt[i].addr, addr, sizeof(ble_addr_t));
            _ppt[i].state = PPT_STATE_FILLED;
            return;
        }
    }
    /* make sure the new scan result is better than the worst existing one */
    ppt_entry_t *pp = _ppt_find_worst();
    assert(pp != NULL);
    if ((pp->ctx.rank > ctx->rank) ||
        ((pp->ctx.rank == ctx->rank) &&
         (pp->ctx.free_slots > ctx->free_slots))) {
        memcpy(&pp->ctx, ctx, sizeof(nimble_netif_rpble_ctx_t));
        memcpy(&pp->addr, addr, sizeof(ble_addr_t));
    }
}

static void _rpble_ad_from_ad(uint8_t *buf, size_t len, nimble_netif_rpble_ctx_t *ctx)
{
    assert(len == (VENDOR_FIELD_LEN - 2));
    ctx->inst_id = buf[POS_INST_ID];
    memcpy(&ctx->dodag_id, &buf[POS_DODAG_ID], 16);
    ctx->version = buf[POS_VERSION];
    ctx->rank = ((int16_t)buf[POS_RANK + 1] << 8) | buf[POS_RANK];  // TODO: proper byteorder
    ctx->free_slots = buf[POS_FREE_SLOTS];
}

static void _make_ad(bluetil_ad_t *ad, uint8_t *buf,
                     const nimble_netif_rpble_ctx_t *ctx, uint8_t free_slots)
{
    /* TODO: make thread save? */
    int res = 0;
    (void)res;

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
    // res = bluetil_ad_add_name(ad, _cfg->name);
    // assert(res == BLUETIL_AD_OK);
}

static void _children_accept(void)
{
    // TODO: update only parts of the AD? */
    /* generate the new advertisement data */
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    _make_ad(&ad, buf, &_local_rpl_ctx,
             (uint8_t)nimble_netif_conn_count(NIMBLE_NETIF_UNUSED) + 1);
    /* start advertising this node. In any case we make sure to use the newest
     * set of advertising data by killing any ongoing advertisements */
    nimble_netif_accept_stop();
    int res = nimble_netif_accept(ad.buf, ad.pos, &_adv_params);
    // TODO remove the following block...
    if (res == NIMBLE_NETIF_OK) {
        DEBUG("# ADV started\n");
    }
    else {
        /* should never happen */
        assert(0);
    }
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
        if (addr->val[5] == 0xea) {
            DEBUG("# ea did not have vend id and right field length\n");
            DEBUG("# len: %i, ven A: 0x%02x B 0x%02x\n", (int)msd_field.len,
                  (int)msd_field.data[0], (int)msd_field.data[1]);
        }
        return;
    }
    nimble_netif_rpble_ctx_t rpble;
    _rpble_ad_from_ad((msd_field.data + 2), (msd_field.len - 2), &rpble);
    _ppt_add(&rpble, addr);
    // DEBUG("# added 0x%02x to PPT\n", (int)addr->val[5]);
}

static void _parent_find(void)
{
    nimble_scanner_start();
    ble_npl_callout_reset(&_evt_eval, _eval_itvl);
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
        DEBUG("# parent_handler: err: unable to connect to preferred parent");
        _parent_find();
    }
    else {
        _current_parent = res;
    }
}

static void _on_netif_evt(int handle, nimble_netif_event_t event)
{
    nimble_netif_conn_t *conn = nimble_netif_conn_get(handle);
    assert(conn);

    switch (event) {
        case NIMBLE_NETIF_CONNECTED_MASTER:
            assert(_current_parent == handle);
            DEBUG("[rpble] PARENT selected: %02x\n", (int)conn->addr[0]);
            _children_accept();
            break;
        case NIMBLE_NETIF_CONNECTED_SLAVE:
            DEBUG("[rpble] CHILD added: %02x\n", (int)conn->addr[0]);
            _children_accept();
            break;
        case NIMBLE_NETIF_CLOSED_MASTER:
            DEBUG("[rpble] PARENT lost: %02x\n", (int)conn->addr[0]);
            _current_parent = PARENT_NONE;
            nimble_netif_accept_stop();
            /* back to 0, now we need to find a new parent... */
            _parent_find();
            break;
        case NIMBLE_NETIF_CLOSED_SLAVE:
            DEBUG("[rpble] CHILD lost %02x\n", (int)conn->addr[0]);
            _children_accept();
            break;
        case NIMBLE_NETIF_CONNECT_ABORT:
            _ppt_remove(conn->addr);
            if (_current_parent == handle) {
                printf("[rpble] PARENT abort (0x%02x)\n", (int)conn->addr[0]);
                _current_parent = PARENT_NONE;
                _parent_find();
            }
            else {
                printf("[rpble] CHILD abort (0x%02x)\n", (int)conn->addr[0]);
                _children_accept();
            }
            break;
        case NIMBLE_NETIF_CONN_UPDATED:
            printf("[rpble] conn param update (0x%02x)\n", (int)conn->addr[0]);
            break;
        default:
            /* should never be reached */
            assert(0);
            break;
    }
}

int nimble_netif_rpble_init(const nimble_netif_rpble_cfg_t *cfg)
{
    assert(cfg);

    /** initialize the eval event */
    ble_npl_time_ms_to_ticks(cfg->eval_itvl, &_eval_itvl);
    ble_npl_callout_init(&_evt_eval, nimble_port_get_dflt_eventq(),
                         _parent_select, NULL);

    _adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    _adv_params.disc_mode = BLE_GAP_DISC_MODE_LTD;
    _adv_params.itvl_min = (cfg->adv_itvl / BLE_HCI_ADV_ITVL);
    _adv_params.itvl_max = (cfg->adv_itvl / BLE_HCI_ADV_ITVL);

    _conn_params.scan_itvl = (cfg->conn_scanitvl / BLE_HCI_SCAN_ITVL);
    _conn_params.scan_window = (cfg->conn_scanwin / BLE_HCI_SCAN_ITVL);
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

    /* start to look for parents */
    _parent_find();

    return NIMBLE_RPBLE_OK;
}

void _parent_find_stop(void)
{
    nimble_scanner_stop();
    ble_npl_callout_stop(&_evt_eval);
    _ppt_clear();
}

int nimble_netif_rpble_update(const nimble_netif_rpble_ctx_t *ctx)
{
    assert(ctx != NULL);

    /* TODO: rather post an update event to the event queue to unify the thread
     * execution context? */

    /* XXX: if the update context is equal to what we have, ignore it */
    _local_rpl_ctx.free_slots = 0;
    if (memcmp(&_local_rpl_ctx, ctx, sizeof(nimble_netif_rpble_ctx_t)) == 0) {
        DEBUG("[rpble] RPL update: ignored (no ctx data change)\n");
        return NIMBLE_RPBLE_NO_CHANGE;
    }

    DEBUG("[rpble] RPL update: got new context data (rank %i, inst %i)\n",
          (int)ctx->rank, (int)ctx->inst_id);

    /* save rpl context for future reference */
    memcpy(&_local_rpl_ctx, ctx, sizeof(nimble_netif_rpble_ctx_t));

    if (ctx->role == GNRC_RPL_ROOT_NODE) {
        _parent_find_stop();
        _current_parent = PARENT_ROOT;
    }

    /* advertise the updated context */
    if (_current_parent != PARENT_NONE) {
        _children_accept();
    }

    return NIMBLE_RPBLE_OK;
}
