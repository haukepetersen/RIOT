
#include "mutex.h"
#include "thread.h"
#include "event/timeout.h"
#include "net/eui48.h"
#include "net/bluetil/ad.h"

#include "nimble_gnrc.h"
#include "host/ble_gap.h"

#include "autoconn.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

/* number of open connections that we want to allow in parallel */
// TODO: move...
#define CONN_NUMOF          (MYNEWT_VAL_BLE_MAX_CONNECTIONS)

// TODO: remove in favor of passed params
#define SCAN_STARTDELAY     (5000000U)  /* in us */
#define SCAN_DURATION       (500000U)   /* in us */
#define SCAN_INTERVAL       (10000000U) /* in us */
#define CONN_WAIT           (1000U)     /* in ms */

#define PRIO                (THREAD_PRIORITY_MAIN - 2)

static char _stack[THREAD_STACKSIZE_DEFAULT];

/* allocate memory for connection contexts */
// TODO: move into connection manager module */
static nimble_gnrc_conn_t _conn_pool[CONN_NUMOF];
static mutex_t _conn_lock = MUTEX_INIT;
static clist_node_t _conn;

static event_queue_t _q;
static event_t _scan_event;
static event_timeout_t _scan_to_event;



// TODO: get these from the config struct
static const struct ble_gap_disc_params _scan_params = { 0 };
static const char *_adv_name = "Mr. Hankey";

static void _scan_start(event_t *event);
static void _scan_stop(event_t *event);

static void _dump_linklocal_addr(const uint8_t *addr)
{
    eui64_t iid;
    eui48_to_ipv6_iid(&iid, (const eui48_t *)addr);
    printf("[fe80:");
    for (unsigned i = 0; i < 8; i += 2) {
        printf(":%02x%02x", (int)iid.uint8[i], (int)iid.uint8[i + 1]);
    }
    printf("]");
}

static void _dump_addr(const uint8_t *addr)
{
    printf("%02x", addr[0]);
    for (unsigned i = 1; i < 6; i++) {
        printf(":%02x", addr[i]);
    }
}

static void _conn_return(nimble_gnrc_conn_t *conn)
{
    mutex_lock(&_conn_lock);
    clist_rpush(&_conn, &conn->node);
    mutex_unlock(&_conn_lock);
}

static nimble_gnrc_conn_t *_conn_get(void)
{
    mutex_lock(&_conn_lock);
    nimble_gnrc_conn_t *conn = (nimble_gnrc_conn_t *)clist_lpop(&_conn);
    mutex_unlock(&_conn_lock);
    return conn;
}

static void _adv_continue(void)
{
    if (nimble_gnrc_get_adv_ctx() != NULL) {
        return;
    }

    nimble_gnrc_conn_t *conn = _conn_get();
    if (conn == NULL) {
        printf("ADV: not advertising: no free context\n");
        return;
    }

    printf("ADV: advertising new context: %p\n", (void *)conn);

    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&ad, _adv_name);
    uint16_t ipss_uuid = BLE_SVC_IPSS;
    bluetil_ad_add(&ad, BLE_GAP_AD_UUID16_COMP, &ipss_uuid, sizeof(uint16_t));

    struct ble_gap_adv_params adv_params = { 0 };
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    int res = nimble_gnrc_accept(conn, ad.buf, ad.pos, &adv_params);
    if (res != NIMBLE_GNRC_OK) {
        _conn_return(conn);
    }
}

// TODO: parse this function from user?
static int _filter(uint8_t *ad, size_t ad_len)
{
    bluetil_ad_t ads = { .buf = ad, .pos = ad_len, .size = ad_len };
    uint16_t ipss = BLE_SVC_IPSS;

    DEBUG("[autoc] running filter\n");

    return bluetil_ad_find_and_cmp(&ads, BLE_GAP_AD_UUID16_COMP,
                                   &ipss, sizeof(ipss));
}

static void _connect(const ble_addr_t *addr)
{
#if ENABLE_DEBUG
    DEBUG("[autoc] connecting to: ");
    nimble_addr_print(addr->val);
    DEBUG("\n");
#endif

    if (nimble_gnrc_get_conn(addr->val) != NULL) {
        DEBUG("     [] abort: already connected\n");
        return;
    }

    nimble_gnrc_conn_t *conn = _conn_get();
    if (conn == NULL) {
        DEBUG("     [] error: no connection slot available\n");
        return;
    }

    if (ble_gap_disc_active() == 1) {
        ble_gap_disc_cancel();
    }

    // TODO: specify connection parameters in module configuration
    int res = nimble_gnrc_connect(conn, addr, NULL, CONN_WAIT);
    if (res != 0) {
        printf("     [] error: unable to initiate connection (%i)\n", res);
        _conn_return(conn);
    }
}


static void _on_blue_evt(nimble_gnrc_conn_t *conn, nimble_gnrc_event_t event)
{
    switch (event) {
        case NIMBLE_GNRC_CONNECTED:
            printf("BLE EVT: CONNECTED %p -> ", (void *)conn);
            _dump_addr(conn->addr);
            printf(" ");
            _dump_linklocal_addr(conn->addr);
            puts("");
            break;
        case NIMBLE_GNRC_DISCONNECTED:
            printf("BLE EVT: DISCONNECTED %p -> ", (void *)conn);
            _dump_addr(conn->addr);
            printf(" ");
            _dump_linklocal_addr(conn->addr);
            puts("");
            _conn_return(conn);
            break;
        case NIMBLE_GNRC_ABORT:
            printf("BLE EVT: conn %p -> ABORT\n", (void *)conn);
            _conn_return(conn);
            break;
        case NIMBLE_GNRC_CONN_UPDATED:
            printf("BLE EVT: conn %p -> UPDATED\n", (void *)conn);
            break;
        default:
            puts("error: unknown connection event\n");
            break;
    }

    /* in any case, make sure we are advertising (if we have a free conn) */
    _adv_continue();
}

static int _on_scan_evt(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_DISC:
            if (_filter(event->disc.data, (size_t)event->disc.length_data)) {
                _connect(&event->disc.addr);
            }
            break;
        case BLE_GAP_EVENT_DISC_COMPLETE:
            /* nothing to be done here */
            break;
        default:
            break;
    }

    return 0;
}

static void _set_scan_start(uint32_t timeout)
{
    _scan_event.handler = _scan_start;
    event_timeout_set(&_scan_to_event, timeout);
}

static void _set_scan_stop(void)
{
    _scan_event.handler = _scan_stop;
    event_timeout_set(&_scan_to_event, SCAN_DURATION);
}

static void _scan_start(event_t *event)
{
    (void)event;

    uint8_t atype;
    // TODO: error checking?!
    // TODO: only actually start scanning if we have available conn memory...

    /* connection procedure active -> skip this scan event */
    if (ble_gap_conn_active() == 0) {
        ble_hs_id_infer_auto(0, &atype);
        ble_gap_disc(atype, BLE_HS_FOREVER, &_scan_params, _on_scan_evt, NULL);
    }
    _set_scan_stop();
}

static void _scan_stop(event_t *event)
{
    (void)event;

    if (ble_gap_disc_active()) {
        ble_gap_disc_cancel();
    }
    _set_scan_start(SCAN_INTERVAL - SCAN_DURATION);
}

static void *_autoconn_thread(void *arg)
{
    (void)arg;

    event_queue_init(&_q);
    _set_scan_start(SCAN_STARTDELAY);
    event_loop(&_q);

    return NULL; /* never reached */
}

// TODO: pass parameters (autoconn_config_t)
int autoconn_run(void)
{
    event_timeout_init(&_scan_to_event, &_q, &_scan_event);

    /* initialize connection list */
    for (unsigned i = 0; i < CONN_NUMOF; i++) {
        memset(&_conn_pool[i], 0, sizeof(nimble_gnrc_conn_t));
        clist_rpush(&_conn, &_conn_pool[i].node);
    }

    _adv_continue();

    /* register event callback */
    nimble_gnrc_eventcb(_on_blue_evt);

    thread_create(_stack, sizeof(_stack), PRIO, THREAD_CREATE_STACKTEST,
                  _autoconn_thread, NULL, "autoconn");

    return AUTOCONN_OK;
}
