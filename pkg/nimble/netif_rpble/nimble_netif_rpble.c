/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     nimble_netif_rpble
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
#include "event/timeout.h"

#include "nimble_netif.h"
#include "nimble_netif_rpble.h"
#include "nimble_scanner.h"
#include "host/ble_gap.h"


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
#define PARENT_NUM          (NIMBLE_NETIF_RPBLE_MAXPARENTS)
#define CHILD_NUM           (CONN_NUMOF - PARENT_NUM)

typedef struct {
    nimble_netif_rpble_ctx_t ctx;      /* TODO: only save rank and free slots? */
    ble_addr_t addr;    /**< potential parents address */
    uint8_t state;      /**< TODO: remove and use existing field implicitly */
} ppt_entry_t;

/* TODO: remove=? */
static const nimble_netif_rpble_cfg_t *_cfg = NULL;

static struct ble_gap_adv_params _adv_params = { 0 };
static struct ble_gap_conn_params _conn_params = { 0 };
static uint32_t _conn_timeout;   /* in ms */

/* local RPL context */
static nimble_netif_rpble_ctx_t _local_rpl_ctx;

/* allocate memory for connection contexts */
static nimble_netif_conn_t _conn_pool[CONN_NUMOF];
static nimble_netif_conn_t *_current_parent = NULL;
static clist_node_t _parents;
static clist_node_t _children;
// TODO: really needed? any call from outside of this threads context?
// -> can be removed once we move all event handlers into the NimBLE thread
static mutex_t _list_lock = MUTEX_INIT;

static ppt_entry_t _ppt[NIMBLE_NETIF_RPBLE_PPTSIZE];

// TODO: remove thread and use nimbles event loop for these events...
#define PRIO                (THREAD_PRIORITY_MAIN - 2)
static char _stack[THREAD_STACKSIZE_DEFAULT];

/* timeout events used for periodical state updates */
/* TODO: make them trickle based? */
static event_queue_t _rpble_q;
static event_t _evt_clear_ppt;
static event_t _evt_eval_parent;
static event_timeout_t _evtto_clear_ppt;
static event_timeout_t _evtto_eval_parent;

static ppt_entry_t *_ppt_find_best(void)
{
    // DEBUG("# find best\n");
    uint16_t rank = UINT16_MAX;
    uint8_t free = 0;
    ppt_entry_t *pp = NULL;
    for (unsigned i = 0; i < NIMBLE_NETIF_RPBLE_PPTSIZE; i++) {
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
    for (unsigned i = 0; i < NIMBLE_NETIF_RPBLE_PPTSIZE; i++) {
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
    for (unsigned i = 0; i < NIMBLE_NETIF_RPBLE_PPTSIZE; i++) {
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
    // for (unsigned i = 0; i < NIMBLE_NETIF_RPBLE_PPTSIZE; i++) {
    //     DEBUG("# ppt[%u]: %02x rank: %i, free_slots: %i\n", i,
    //           (int)_ppt[i].addr.val[5],
    //           (int)_ppt[i].ctx.rank,
    //           (int)_ppt[i].ctx.free_slots);
    // }

    /* scan table if given address is already part of it. If so, update entry */
    for (unsigned i = 0; i < NIMBLE_NETIF_RPBLE_PPTSIZE; i++) {
        if (memcmp(addr, &_ppt[i].addr, sizeof(ble_addr_t)) == 0) {
            // DEBUG("# PPT match\n");
            memcpy(&_ppt[i].ctx, ctx, sizeof(nimble_netif_rpble_ctx_t));
            return;
        }
    }
    /* if not found, look for an empty slot */
    for (unsigned i = 0; i < NIMBLE_NETIF_RPBLE_PPTSIZE; i++) {
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

static void _conn_return(clist_node_t *list, nimble_netif_conn_t *conn)
{
    mutex_lock(&_list_lock);
    clist_rpush(list, &conn->node);
    mutex_unlock(&_list_lock);
}

static nimble_netif_conn_t *_conn_get(clist_node_t *list)
{
    mutex_lock(&_list_lock);
    nimble_netif_conn_t *conn = (nimble_netif_conn_t *)clist_lpop(list);
    mutex_unlock(&_list_lock);
    return conn;
}

static void _stop_advertising(void)
{
    nimble_netif_conn_t *conn = nimble_netif_get_adv_ctx();
    if (conn) {
        nimble_netif_terminate(conn);
        _conn_return(&_children, conn);
        DEBUG("# advertise stopped\n");
    }
}

static void _try_to_advertise(void)
{
    nimble_netif_conn_t *conn = _conn_get(&_children);
    if (conn != NULL) {
        DEBUG("# advertising now\n");
        /* generate the new advertisement data */
        uint8_t buf[BLE_HS_ADV_MAX_SZ];
        bluetil_ad_t ad;
        _make_ad(&ad, buf, &_local_rpl_ctx,
                 (uint8_t)(clist_count(&_children) + 1));

        // DEBUG("# prepared AD (pos: %i):\n", (int)ad.pos);
        // for (size_t i = 0; i < ad.pos; i++) {
        //     DEBUG("0x%02x ", (int)ad.buf[i]);
        // }
        // puts("");
        int res = nimble_netif_accept(conn, ad.buf, ad.pos, &_adv_params);
        assert(res == NIMBLE_NETIF_OK);
        (void)res;
    }
}

static void _on_scan_evt(const ble_addr_t *addr, int8_t rssi,
                         const uint8_t *ad, size_t ad_len)
{
    int res;

    // if (addr->val[5] == 0xea) {
    //     DEBUG("# seen node 0x%02x %02x\n", (int)addr->val[5], (int)addr->val[0]);
    //     for (size_t i = 0; i < ad_len; i++) {
    //         DEBUG("0x%02x ", (int)ad[i]);
    //     }
    //     puts("");
    // }

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

static void _on_netif_evt(nimble_netif_conn_t *conn, nimble_netif_event_t event)
{
    assert(conn);

    switch (event) {
        case NIMBLE_NETIF_CONNECTED: {
            if (_current_parent == conn) {
                DEBUG("[rpble] PARENT selected: %02x\n",
                      (int)conn->addr[5]);
                nimble_scanner_start();
            }
            else {
                DEBUG("[rpble] CHILD added: %02x\n",
                      (int)conn->addr[5]);
                _try_to_advertise();
            }
            break;
        }
        case NIMBLE_NETIF_DISCONNECTED:
            if (_current_parent == conn) {
                _current_parent = NULL;
                _ppt_remove(conn->addr);
                _conn_return(&_parents, conn);
                _stop_advertising();
                /* try to connect to the next fitting parent immediately */
                event_post(&_rpble_q, &_evt_eval_parent);
                DEBUG("[rpble] PARENT lost: %02x\n", (int)conn->addr[5]);
            }
            else {
                _conn_return(&_children, conn);
                if (_current_parent || (_local_rpl_ctx.role == 1)) {
                    _stop_advertising();
                    _try_to_advertise();
                }
                DEBUG("[rpble] CHILD lost %02x\n", (int)conn->addr[5]);
            }
            break;
        case NIMBLE_NETIF_ABORT:

            /* TODO: remove failed peer from PPT! */

            if (_current_parent == conn) {
                _current_parent = NULL;
                _conn_return(&_parents, conn);
                DEBUG("[rpble] conn abort (master 0x%02x)\n", (int)conn->addr[5]);
                nimble_scanner_start();
            }
            else {
                _conn_return(&_children, conn);
                DEBUG("[rpble] conn abort (child 0x%02x)\n", (int)conn->addr[5]);
            }
            break;
            break;
        case NIMBLE_NETIF_CONN_UPDATED:
            printf("[rpble] conn param update (0x%02x)\n", (int)conn->addr[5]);
            break;
        default:
            /* should never be reached */
            assert(0);
            break;
    }
}

static void _clear_ppt_handler(event_t *event)
{
    (void)event;
    _ppt_clear();
    event_timeout_set(&_evtto_clear_ppt, _cfg->clear_timeout);
}

static void _eval_parent_handler(event_t *event)
{
    (void)event;
    int res;
    (void)res;

    /* reset timer  */
    event_timeout_clear(&_evtto_eval_parent);
    if (_local_rpl_ctx.role != 1) {
        event_timeout_set(&_evtto_eval_parent, _cfg->eval_timeout);
    }
    else {
        DEBUG("# eval: we are root, so no more parent evalutations\n");
        return;
    }

    ppt_entry_t *pp = _ppt_find_best();
    if (pp == NULL) {
        // DEBUG("# no fitting parent in PPT\n");
        return;
    }
    if (_current_parent) {
        /* if we do have an active parent, and pp is not equal to that parent */
        if ((memcmp(&_current_parent->addr,
                           pp->addr.val, sizeof(BLE_ADDR_LEN)) != 0)) {
            DEBUG("# we have a parent, but we want a better one -> disonn\n");
            nimble_netif_terminate(_current_parent);
        }
    }
    else {
        DEBUG("# parent selected. will connect now: %02x\n", (int)pp->addr.val[5]);
        _current_parent = _conn_get(&_parents);
        assert(_current_parent != NULL);
        nimble_scanner_stop();
        res = nimble_netif_connect(_current_parent, &pp->addr,
                                   &_conn_params, _conn_timeout);
        assert(res == NIMBLE_NETIF_OK);
    }
}

static void *_run(void *arg)
{
    /* initialize event loop and the events that we use */
    event_queue_init(&_rpble_q);
    _evt_clear_ppt.handler = _clear_ppt_handler;
    _evt_eval_parent.handler = _eval_parent_handler;
    event_timeout_init(&_evtto_clear_ppt, &_rpble_q, &_evt_clear_ppt);
    event_timeout_init(&_evtto_eval_parent, &_rpble_q, &_evt_eval_parent);

    event_timeout_set(&_evtto_clear_ppt, _cfg->clear_timeout);
    event_timeout_set(&_evtto_eval_parent, _cfg->eval_timeout);

    /* handle events here for now (TODO: move to using NimBLE's event loop) */
    event_loop(&_rpble_q);

    /* never reached (in theory) */
    return NULL;
}


int nimble_netif_rpble_init(const nimble_netif_rpble_cfg_t *cfg)
{
    assert(cfg);

    _cfg = cfg;

    /* initialize connection list */
    for (unsigned i = 0; i < CONN_NUMOF; i++) {
        memset(&_conn_pool[i], 0, sizeof(nimble_netif_conn_t));
        if (i < PARENT_NUM) {
            clist_rpush(&_parents, &_conn_pool[i].node);
        }
        else {
            clist_rpush(&_children, &_conn_pool[i].node);
        }
    }

    /* apply configuration parameters */
    struct ble_gap_disc_params scan_params = { 0 };
    scan_params.itvl = (cfg->scan_itvl / BLE_HCI_SCAN_ITVL);
    scan_params.window = (cfg->scan_win / BLE_HCI_SCAN_ITVL);
    scan_params.passive = 1;
    scan_params.filter_duplicates = 1;

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

    // DEBUG("[rpble] CONNCTION PARAMS\n");
    // DEBUG("init: adv itvl: %i\n", (int)_adv_params.itvl_min);
    // DEBUG("init: scan itvl: %i\n", (int)_conn_params.scan_itvl);
    // DEBUG("init: conn itvl: %i\n", (int)_conn_params.itvl_min);
    // DEBUG("init: spvn to  : %i\n", (int)_conn_params.supervision_timeout);
    // DEBUG("init: _conn_timeout: %i\n", (int)_conn_timeout);

    /* register event callback */
    nimble_netif_eventcb(_on_netif_evt);
    /* configure scanner */
    nimble_scanner_init(&scan_params, _on_scan_evt);


    // TODO remove and use nimble's event loop!
    thread_create(_stack, sizeof(_stack), PRIO, THREAD_CREATE_STACKTEST,
                  _run, NULL, "nimble_rpble");

    /* start scanning */
    nimble_scanner_start();
    DEBUG("[rpble] scanning now\n");

    return NIMBLE_NETIF_RPBLE_OK;
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
        return NIMBLE_NETIF_RPBLE_NO_CHANGE;
    }

    DEBUG("[rpble] RPL update: got new context data (rank %i, inst %i)\n",
          (int)ctx->rank, (int)ctx->inst_id);

    if (ctx->role == 1) {
        nimble_scanner_stop();
        _ppt_clear();
    }

    /* stop any ongoing advertisements */
    _stop_advertising();
    /* save rpl context for future reference */
    memcpy(&_local_rpl_ctx, ctx, sizeof(nimble_netif_rpble_ctx_t));
    /* and start to advertise */
    _try_to_advertise();

    return NIMBLE_NETIF_RPBLE_OK;
}
