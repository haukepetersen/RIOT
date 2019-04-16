
#include <stdlib.h>

#include "clist.h"
#include "mutex.h"
#include "xtimer.h"
#include "nimble_netif.h"
#include "net/bluetil/ad.h"
#include "net/bluetil/addr.h"

#include "nimble_scanlist.h"
#include "nimble_scanner.h"

#include "app.h"

static nimble_netif_conn_t _conn_mem[NIMBLE_MAX_CONN];
static clist_node_t _conn_pool;
static clist_node_t _conn_active;
static nimble_netif_conn_t *_conn_adv = NULL;
static mutex_t _conn_lock = MUTEX_INIT;

/* save the advertising configuration statically */
static uint8_t _adbuf[BLE_HS_ADV_MAX_SZ];
static bluetil_ad_t _ad;
static const struct ble_gap_adv_params _adv_params = {
    .conn_mode = BLE_GAP_CONN_MODE_UND,
    .disc_mode = BLE_GAP_DISC_MODE_LTD,
    .itvl_min = APP_ADV_ITVL,
    .itvl_max = APP_ADV_ITVL,
};

#define SCAN_DUR_DEFAULT        (500U)      /* 500ms */
#define CONN_TIMEOUT            (500U)      /* 500ms */


static nimble_netif_conn_t *_conn_next(clist_node_t *list,
                                       nimble_netif_conn_t *conn)
{
    if (conn == NULL) {
        return (nimble_netif_conn_t *)list->next;
    }
    else {
        conn = (nimble_netif_conn_t *)conn->node.next;
        if (conn == (nimble_netif_conn_t *)list->next) {
            return NULL;    /* end of list */
        }
    }
    return conn;
}

static nimble_netif_conn_t *_conn_get(clist_node_t *list)
{
    mutex_lock(&_conn_lock);
    nimble_netif_conn_t *conn = (nimble_netif_conn_t *)clist_lpop(list);
    mutex_unlock(&_conn_lock);
    return conn;
}

static void _conn_add(clist_node_t *list, nimble_netif_conn_t *conn)
{
    mutex_lock(&_conn_lock);
    clist_rpush(list, &conn->node);
    mutex_unlock(&_conn_lock);
}

static void _conn_remove(clist_node_t *list, nimble_netif_conn_t *conn)
{
    mutex_lock(&_conn_lock);
    clist_remove(list, &conn->node);
    mutex_unlock(&_conn_lock);
}

static nimble_netif_conn_t *_conn_by_pos(clist_node_t *list, unsigned pos)
{
    unsigned i = 0;
    nimble_netif_conn_t *conn = NULL;

    mutex_lock(&_conn_lock);
    do {
        conn = _conn_next(list, conn);
        i++;
    } while (conn && (i <= pos));
    mutex_unlock(&_conn_lock);

    return conn;
}

static void _on_ble_evt(nimble_netif_conn_t *conn, nimble_netif_event_t event)
{
    switch (event) {
        case NIMBLE_NETIF_CONNECTED_MASTER: {
            printf("event: ");
            bluetil_addr_print(conn->addr);
            puts(" -> CONNECTED as MASTER");
            _conn_add(&_conn_active, conn);
            /* connection sequence is complete, so continue advertising (if
             * applicable) */
            mutex_lock(&_conn_lock);

            if (_conn_adv) {
                int res = nimble_netif_accept(_conn_adv, _ad.buf, _ad.pos,
                                              &_adv_params);
                assert(res == NIMBLE_NETIF_OK);
                (void)res;
            }
            mutex_unlock(&_conn_lock);
            break;
        }
        case NIMBLE_NETIF_CONNECTED_SLAVE:
            printf("event: ");
            bluetil_addr_print(conn->addr);
            puts(" -> CONNECTED as SLAVE");
            mutex_lock(&_conn_lock);
            assert(conn == _conn_adv);
            _conn_adv = NULL;
            mutex_unlock(&_conn_lock);
            _conn_add(&_conn_active, conn);
            break;
        case NIMBLE_NETIF_DISCONNECTED:
            printf("event: ");
            bluetil_addr_print(conn->addr);
            puts(" -> CONNECTION CLOSED");
            _conn_remove(&_conn_active, conn);
            _conn_add(&_conn_pool, conn);
            break;
        case NIMBLE_NETIF_CONNECT_ABORT:
            printf("event: CONNECTION ABORT\n");
            _conn_add(&_conn_pool, conn);
            break;
        case NIMBLE_NETIF_ACCEPT_ABORT:
            puts("event: stopped advertising");
            mutex_lock(&_conn_lock);
            assert(conn == _conn_adv);
            _conn_adv = NULL;
            mutex_unlock(&_conn_lock);
            _conn_add(&_conn_pool, conn);
            break;
        case NIMBLE_NETIF_CONN_UPDATED:
        default:
            /* do nothing */
            break;
    }
}
static void _conn_list(void)
{
    unsigned i = 0;
    nimble_netif_conn_t *conn = _conn_next(&_conn_active, NULL);
    const char roles[2] = { 'M', 'S' };

    while (conn) {
        printf("[%2u] ", i++);
        bluetil_addr_print(conn->addr);
        printf(" (%c) -> ", roles[conn->role]);
        bluetil_addr_ipv6_l2ll_print(conn->addr);
        puts("");
        conn = _conn_next(&_conn_active, conn);
    }
}

static void _cmd_info(void)
{
    puts("Connection status:");

    mutex_lock(&_conn_lock);
    unsigned free = (unsigned)clist_count(&_conn_pool);
    unsigned active = (unsigned)clist_count(&_conn_active);

    printf(" Free slots: %u/%u\n", free, NIMBLE_MAX_CONN);
    printf("Advertising: ");
    if (_conn_adv != NULL) {
        puts("yes");
    }
    else {
        puts("no");
    }
    if (active > 0) {
        printf("%u established connections:\n", active);
        _conn_list();
    }
    mutex_unlock(&_conn_lock);
    puts("");
}

static void _cmd_adv(const char *name)
{
    int res;

    /* make sure no advertising is in progress */
    mutex_lock(&_conn_lock);
    int active = (_conn_adv != NULL);
    mutex_unlock(&_conn_lock);
    if (active) {
        puts("err: advertising already in progress");
        return;
    }

    /* build advertising data */
    res = bluetil_ad_init_with_flags(&_ad, _adbuf, BLE_HS_ADV_MAX_SZ,
                                         BLUETIL_AD_FLAGS_DEFAULT);
    assert(res == BLUETIL_AD_OK);
    uint16_t ipss = BLE_SVC_IPSS;
    res = bluetil_ad_add(&_ad, BLE_GAP_AD_UUID16_INCOMP, &ipss, sizeof(ipss));
    assert(res == BLUETIL_AD_OK);
    if (name == NULL) {
        name = APP_ADV_NAME_DEFAULT;
    }
    res = bluetil_ad_add(&_ad, BLE_GAP_AD_NAME, name, strlen(name));
    if (res != BLUETIL_AD_OK) {
        puts("err: the given name is too long");
        return;
    }

    /* get free connection context */
    nimble_netif_conn_t *conn = _conn_get(&_conn_pool);
    if (conn == NULL) {
        puts("err: connection limit maxed out\n");
        return;
    }

    /* start listening for incoming connections */
    res = nimble_netif_accept(conn, _ad.buf, _ad.pos, &_adv_params);
    if (res != NIMBLE_NETIF_OK) {
        _conn_add(&_conn_pool, conn);
        printf("err: unable to start advertising (%i)\n", res);
    }
    else {
        _conn_adv = conn;
        printf("success: advertising this node: '%s'\n", name);
    }
}

static void _cmd_adv_stop(void)
{
    int res = nimble_netif_accept_stop();
    if (res == NIMBLE_NETIF_OK) {
        puts("canceled advertising");
    }
    else if (res == NIMBLE_NETIF_NOTADV) {
        puts("err: no advertising in progress");
    }
}

static void _cmd_scan(unsigned duration)
{
    if (duration == 0) {
        return;
    }
    printf("scanning (for %ums) ... ", (duration / 1000));
    nimble_scanlist_clear();
    nimble_scanner_start();
    xtimer_usleep(duration);
    nimble_scanner_stop();
    puts("done");
    nimble_scanlist_print();
}

static void _cmd_connect(unsigned pos)
{
    nimble_scanlist_entry_t *sle = nimble_scanlist_get_by_pos(pos);
    if (sle == NULL) {
        puts("err: unable to find given offset in scanlist");
        return;
    }
    nimble_netif_conn_t *conn = _conn_get(&_conn_pool);
    if (conn == NULL) {
        puts("err: connection limit already maxed out\n");
        return;
    }
    /* we need to stop accepting before connecting */
    _cmd_adv_stop();

    /* TODO: for now we use default parameters */
    int res = nimble_netif_connect(conn, &sle->addr, NULL, CONN_TIMEOUT);
    if (res != NIMBLE_NETIF_OK) {
        _conn_add(&_conn_pool, conn);
        printf("err: unable to trigger connection sequence (%i)\n", res);
        return;
    }

    printf("trying to connect to ");
    bluetil_addr_print(sle->addr.val);
    puts("");
}

static void _cmd_conn_list(void)
{
    mutex_lock(&_conn_lock);
    _conn_list();
    mutex_unlock(&_conn_lock);
}

static void _cmd_close(unsigned chan)
{
    nimble_netif_conn_t *conn = _conn_by_pos(&_conn_active, chan);
    if (conn == NULL) {
        puts("err: can't find connection handle");
        return;
    }
    int res = nimble_netif_disconnect(conn);
    if (res != NIMBLE_NETIF_OK) {
        puts("err: unable to close given connection");
        return;
    }
}

static int _ishelp(char *argv)
{
    return memcmp(argv, "help", 4) == 0;
}

void app_ble_init(void)
{
    /* initialize connection pool */
    memset(_conn_mem, 0, sizeof(_conn_mem));
    for (unsigned i = 0; i < (sizeof(_conn_mem) / sizeof(_conn_mem[0])); i++) {
        clist_rpush(&_conn_pool, &_conn_mem[i].node);
    }

    /* setup the scanning environment */
    nimble_scanlist_init();
    nimble_scanner_init(NULL, nimble_scanlist_update);

    /* register event callback with the netif wrapper */
    nimble_netif_eventcb(_on_ble_evt);
}

int app_ble_cmd(int argc, char **argv)
{
    if ((argc == 1) || _ishelp(argv[1])) {
        printf("usage: %s [help|info|adv|scan|connect|close]\n", argv[0]);
        return 0;
    }
    if ((memcmp(argv[1], "info", 4) == 0)) {
        _cmd_info();
    }
    else if ((memcmp(argv[1], "adv", 3) == 0)) {
        char *name = NULL;
        if (argc > 2) {
            if (_ishelp(argv[2])) {
                printf("usage: %s adv [help|stop|<name>]\n", argv[0]);
                return 0;
            }
            if (memcmp(argv[2], "stop", 4) == 0) {
                _cmd_adv_stop();
                return 0;
            }
            name = argv[2];
        }
        _cmd_adv(name);
    }
    else if ((memcmp(argv[1], "scan", 4) == 0)) {
        uint32_t duration = SCAN_DUR_DEFAULT;
        if (argc > 2) {
            if (_ishelp(argv[2])) {
                printf("usage: %s scan [help|list|[duration in ms]]\n", argv[0]);
                return 0;
            }
            if (memcmp(argv[2], "list", 4) == 0) {
                nimble_scanlist_print();
                return 0;
            }
            duration = (uint32_t)atoi(argv[2]);
        }
        _cmd_scan(duration * 1000);
    }
    else if ((memcmp(argv[1], "connect", 7) == 0)) {
        if ((argc < 3) || _ishelp(argv[2])) {
            printf("usage: %s connect [help|list|<scanlist entry #>]\n", argv[0]);
        }
        if (memcmp(argv[2], "list", 4) == 0) {
            _cmd_conn_list();
            return 0;
        }
        unsigned pos = (unsigned)atoi(argv[2]);
        _cmd_connect(pos);
    }
    else if ((memcmp(argv[1], "close", 5) == 0)) {
        if ((argc < 3) || _ishelp(argv[2])) {
            printf("usage: %s close [help|list|<conn #>]\n", argv[0]);
            return 0;
        }
        if (memcmp(argv[2], "list", 4) == 0) {
            _cmd_conn_list();
            return 0;
        }
        unsigned chan = (unsigned)atoi(argv[2]);
        _cmd_close(chan);
    }
    else {
        printf("unable to parse the command. Use '%s help' for more help\n",
               argv[0]);
    }

    return 0;
}
