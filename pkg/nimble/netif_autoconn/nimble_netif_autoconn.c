
#include "mutex.h"
#include "thread.h"
#include "random.h"
#include "event/timeout.h"
#include "net/eui48.h"
#include "net/bluetil/ad.h"
#include "net/bluetil/addr.h"

#include "nimble_netif.h"
#include "nimble_netif_autoconn.h"
#include "nimble_scanner.h"
#include "host/ble_gap.h"


#define ENABLE_DEBUG        (1)
#include "debug.h"

#define CCNL_UUID           (0x2342)

/* number of open connections that we want to allow in parallel */
#define CONN_NUMOF          (MYNEWT_VAL_BLE_MAX_CONNECTIONS)

static const nimble_netif_autoconn_cfg_t *_cfg = NULL;

static struct ble_gap_adv_params _adv_params = { 0 };
static struct ble_gap_conn_params _conn_params = { 0 };
static uint32_t _conn_timeout;   /* in ms */

/* allocate memory for connection contexts */
static nimble_netif_conn_t _conn_pool[CONN_NUMOF];
static clist_node_t _conn;

// TODO: really needed? any call from outside of this threads context?
static mutex_t _conn_lock = MUTEX_INIT;

/* buffer the used advertisement data */
static uint8_t _ad_buf[BLE_HS_ADV_MAX_SZ];
static bluetil_ad_t _ad;

/* keep the current state */
static nimble_netif_conn_t *_adv_conn = NULL;
static nimble_netif_conn_t *_connecting_conn = NULL;
static nimble_netif_conn_t *_scanning_conn = NULL;

// TODO: remove thread and use nimbles event loop for these events...
#define PRIO                (THREAD_PRIORITY_MAIN - 2)
static char _stack[THREAD_STACKSIZE_DEFAULT];


static void _conn_return(nimble_netif_conn_t *conn)
{
    mutex_lock(&_conn_lock);
    clist_rpush(&_conn, &conn->node);
    mutex_unlock(&_conn_lock);
}

static nimble_netif_conn_t *_conn_get(void)
{
    mutex_lock(&_conn_lock);
    nimble_netif_conn_t *conn = (nimble_netif_conn_t *)clist_lpop(&_conn);
    mutex_unlock(&_conn_lock);
    return conn;
}

// static bool _conn_avail(void)
// {
//     return clist_lpeek(&_conn) != NULL;
// }

static void _on_scan_evt(const ble_addr_t *addr, int8_t rssi,
                         const uint8_t *ad, size_t ad_len)
{
    // printf("[autoconn] scan evt: ");
    // bluetil_addr_print(addr->val);
    // puts("");

    /* check autoconn's state */
    if (_connecting_conn) {
        DEBUG("        [] WARNING: scan event while already connecting\n");
        nimble_scanner_stop();
        return;
    }
    if (_scanning_conn == NULL) {
        DEBUG("        [] WARNING: scan event w/o connection context\n");
        nimble_scanner_stop();
        return;
    }

    /* make sure our filter criteria are met */
#ifdef MODULE_NIMBLE_NETIF_AUTOCONN_IPSP
    uint16_t uuid = BLE_SVC_IPSS;
#endif
#ifdef MODULE_NIMBLE_NETIF_AUTOCONN_CCNL
    uint16_t uuid = CCNL_UUID;
#endif
    bluetil_ad_t ads = { .buf = (uint8_t *)ad, .pos = ad_len, .size = ad_len };
    if (!bluetil_ad_find_and_cmp(&ads, BLE_GAP_AD_UUID16_COMP,
                                &uuid, sizeof(uuid))) {
        return;
    }

    /* are we already connected to the node? */
    if (nimble_netif_get_conn(addr->val) != NULL) {
        return;
    }

    /* now lets initiate the connection attempt */
    nimble_scanner_stop();
    DEBUG("[autoconn] INITIATING connection to ");
    bluetil_addr_print(addr->val);
    DEBUG("\n");

    int res = nimble_netif_connect(_scanning_conn, addr, &_conn_params, _conn_timeout);
    if (res != NIMBLE_NETIF_OK) {
        DEBUG("        [] ERROR: failed trigger netif_connect (%i)\n", res);
        _conn_return(_scanning_conn);
        _scanning_conn = NULL;
    }
    else {
        _connecting_conn = _scanning_conn;
        _scanning_conn = NULL;
    }
}

static void _on_netif_evt(nimble_netif_conn_t *conn, nimble_netif_event_t event)
{
    assert(conn);

    switch (event) {
        case NIMBLE_NETIF_CONNECTED: {
            printf("BLE EVT: CONNECTED %p -> ", (void *)conn);
            bluetil_addr_print(conn->addr);
            printf(" ");
            bluetil_addr_print_ipv6_iid(conn->addr);
            puts("");

            assert(conn != _scanning_conn);
            if (conn == _connecting_conn) {
                DEBUG("[autoconn] INITIATING connection success\n");
                _connecting_conn = NULL;
            } else if (conn == _adv_conn) {
                DEBUG("[autoconn] ACCEPTING connection success\n");
                assert(nimble_netif_get_adv_ctx() == NULL);
                _adv_conn = NULL;
            }
            else {
                assert(0);
            }

            break;
        }
        case NIMBLE_NETIF_DISCONNECTED:
            printf("BLE EVT: DISCONNECTED %p -> ", (void *)conn);
            bluetil_addr_print(conn->addr);
            printf(" ");
            bluetil_addr_print_ipv6_iid(conn->addr);
            puts("");

            assert(conn != _connecting_conn);
            assert(conn != _adv_conn);
            assert(conn != _scanning_conn);
            _conn_return(conn);
            break;
        case NIMBLE_NETIF_ABORT:
            printf("BLE EVT: conn %p -> ABORT\n", (void *)conn);

            assert(_scanning_conn == NULL);
            if (conn == _connecting_conn) {
                DEBUG("[autoconn] INITIATING connection aborted\n");
                _connecting_conn = NULL;
            }
            else if (conn == _adv_conn) {
                DEBUG("[autoconn] ACCEPTING connection aborted\n");
                _adv_conn = NULL;
            }
            else {
                assert(0);
            }

            _conn_return(conn);
            break;
        case NIMBLE_NETIF_CONN_UPDATED:
            printf("BLE EVT: conn %p -> UPDATED\n", (void *)conn);
            break;
        default:
            puts("error: unknown connection event\n");
            break;
    }
}

static void _waitfor_period(uint32_t base)
{
    uint32_t delay = random_uint32_range(0, _cfg->period_jitter);
    xtimer_usleep(base + delay);
}

static void *_run(void *arg)
{
    /* startup delay, 1-2 jitter periods */
    _waitfor_period(_cfg->period_jitter);

    while (1) {
        /* scan first, advertise later :-) */

        /* skip event if we are trying to connect to something */
        printf("[autoconn] SCAN");
        if (_connecting_conn == NULL) {
            DEBUG(" -> go\n");

            _scanning_conn = _conn_get();
            if (_scanning_conn) {
                int res = nimble_scanner_start();
                if (res != NIMBLE_SCANNER_OK) {
                    printf("        [] ERROR: failed to start scanner\n");
                    _conn_return(_scanning_conn);
                    _scanning_conn = NULL;
                }
            }
        }
        else {
            DEBUG(" -> skip\n");
        }

        _waitfor_period(_cfg->period_scan);

        /* finish up */
        if (_scanning_conn) {
            assert(_connecting_conn == NULL);
            nimble_scanner_stop();
            _conn_return(_scanning_conn);
            _scanning_conn = NULL;
        }

        /* if not connecting, go over to advertising this node */
        DEBUG("[autoconn] ADV");
        if (_connecting_conn == NULL) {
            DEBUG(" -> go\n");

            _adv_conn = _conn_get();
            if (_adv_conn) {
                int res = nimble_netif_accept(_adv_conn, _ad.buf, _ad.pos, &_adv_params);
                if (res != 0) {
                    printf("        [] ERROR: failed to start advertising\n");
                    _conn_return(_adv_conn);
                    _adv_conn = NULL;
                }
            }

        }
        else {
            DEBUG(" -> skip\n");
        }

        _waitfor_period(_cfg->period_adv);

        if (_adv_conn) {
            assert(_connecting_conn == NULL);
            if (_adv_conn == nimble_netif_get_adv_ctx()) {
                nimble_netif_terminate(_adv_conn);
                _conn_return(_adv_conn);
                _adv_conn = NULL;
            }
            /* verify ctx, TODO: remove?! */
            else {
                DEBUG("[autoconn] ERROR: _adv_conn != get_adv_ctx()");
                assert(0);
            }
        }
    }

    /* never reached */
    return NULL;
}


int nimble_netif_autoconn_init(const nimble_netif_autoconn_cfg_t *cfg)
{
    assert(cfg);

    _cfg = cfg;

    /* initialize connection list */
    for (unsigned i = 0; i < CONN_NUMOF; i++) {
        memset(&_conn_pool[i], 0, sizeof(nimble_netif_conn_t));
        clist_rpush(&_conn, &_conn_pool[i].node);
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

    DEBUG("[autoconn] CONNCTION PARAMS\n");
    DEBUG("init: adv itvl: %i\n", (int)_adv_params.itvl_min);
    DEBUG("init: scan itvl: %i\n", (int)_conn_params.scan_itvl);
    DEBUG("init: conn itvl: %i\n", (int)_conn_params.itvl_min);
    DEBUG("init: spvn to  : %i\n", (int)_conn_params.supervision_timeout);
    DEBUG("init: _conn_timeout: %i\n", (int)_conn_timeout);


    /* prepare advertisement data */
    bluetil_ad_init_with_flags(&_ad, _ad_buf, sizeof(_ad_buf),
                               BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&_ad, cfg->name);
#ifdef MODULE_NIMBLE_NETIF_AUTOCONN_IPSP
    uint16_t ipss_uuid = BLE_SVC_IPSS;
    bluetil_ad_add(&_ad, BLE_GAP_AD_UUID16_COMP, &ipss_uuid, sizeof(uint16_t));
#endif

    /* register event callback */
    nimble_netif_eventcb(_on_netif_evt);
    /* configure scanner */
    nimble_scanner_init(&scan_params, _on_scan_evt, NULL);


    // TODO remove and use nimble's event loop!
    thread_create(_stack, sizeof(_stack), PRIO, THREAD_CREATE_STACKTEST,
                  _run, NULL, "nimble_autoconn");

    return NIMBLE_NETIF_AUTOCONN_OK;
}

size_t nimble_netif_autoconn_free_slots(void)
{
    size_t res;
    mutex_lock(&_conn_lock);
    res = clist_count(&_conn);
    mutex_unlock(&_conn_lock);
    return res;
}
