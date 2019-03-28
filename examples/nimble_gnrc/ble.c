
#include <stdlib.h>

#include "clist.h"
#include "mutex.h"
#include "xtimer.h"
#include "nimble_netif.h"

#include "nimble_scanlist.h"
#include "nimble_scanner.h"

static nimble_netif_conn_t _conn_mem[NIMBLE_MAX_CONN];
static clist_node_t _conn_pool;
static mutex_t _conn_lock = MUTEX_INIT;

#define SCAN_DUR_DEFAULT        (500U)      /* 500ms */

static nimble_netif_conn_t *_conn_next(clist_node_t list,
                                       nimble_netif_conn_t *conn)
{
    if (conn == NULL) {
        return (nimble_netif_conn_t *)list->next;
    }
    else {
        conn = (nimble_netif_conn_t *)conn->node.next;
        (conn == (nimble_netif_conn_t *)list.next)
            ? return NULL;    /* end of list */
            : return conn;
    }
}

static nimble_netif_conn_t *_conn_get(clist_node_t *list)
{
    mutex_lock(&_conn_lock);
    nimble_netif_conn_t *conn = (nimble_netif_conn_t *)clist_lpop(&_conn_pool);
    mutex_unlock(&_conn_lock);
    return conn;
}

static void _conn_add(clist_node_t *list, nimble_netif_con_t *conn)
{
    mutex_lock(&_conn_lock);
    clist_rpush(&conn->node);
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
        case NIMBLE_NETIF_CONNECTED:
            printf("event: ");
            bluetil_addr_print(conn->addr);
            puts(" -> CONNECTED");
            _conn_add(&_conn_active, conn);
            nimble_netif_accept_resume();
            break;
        case NIMBLE_NETIF_DISCONNECTED:
            printf("event: ");
            bluetil_addr_print(conn->addr);
            puts(" -> CONNECTION CLOSED");
            _conn_add(&_conn_pool, conn);
            break;
        case NIMBLE_NETIF_CONNECT_ABORT:

            break;

        case NIMBLE_NETIF_ACCEPT_ABORT:
            puts("event: stopped advertising")
            break;
        case NIMBLE_NETIF_CONN_UPDATED:
        default:
            /* do nothing */
            break;
    }
}

static void _cmd_info(void)
{
    puts("INFO");
}

static void _cmd_adv(const char *name)
{
    if (name) {
        printf("ADVERTISING %s\n", name);
    }
    else {
        puts("ADV default name");
    }
}

static void _cmd_adv_stop(void)
{
    int res = nimble_netif_stop_accepting();
    assert(res == NIMBLE_NETIF_OK);
    (void)res;
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
    nimble_scanlist_entry_t *sle = nimble_scanlist_get(pos);
    if (sle == NULL) {
        puts("err: unable to find given offset in scanlist");
        return;
    }
    nimble_netif_conn_t *conn = _conn_get(&_conn_pool);
    if (conn == NULL) {
        puts("err: connection limit already maxed out\n");
        return;
    }
    /* TODO: we need to stop accepting before connecting */
    int res = nimble_netif_accept_pause();
    assert(res == NIMBLE_NETIF_OK);

    res = nimble_netif_connect(conn, sle->addr, _conn_params, _conn_timeout);
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
    unsigned i = 0;

    mutex_lock(&_conn_lock);
    nimble_netif_conn_t *conn = _conn_next(&_conn_active, NULL);
    while (conn) {
        printf("[%2u] ");
        bluetil_addr_print(conn->addr);
        puts("");
        conn = _conn_next(&_conn_active, conn);
    }
    mutex_unlock(&_conn_lock);
}

static void _cmd_close(unsigned chan)
{
    nimble_netif_conn_t *conn = _conn_by_pos(chan);
    if (conn == NULL) {
        puts("err: can't find connection handle");
        return;
    }
    int res = nimble_netif_terminat(conn);
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
                printf("usage: %s adv [help|stop|<name>]", argv[0]);
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
                printf("usage: %s scan [help|list|<duration in ms>]", argv[0]);
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
            printf("usage: %s connect [help|list|<scanlist entry #>]", argv[0]);
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
            printf("usage: %s scan [help|<conn #>]", argv[0]);
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
