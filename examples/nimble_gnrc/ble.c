
#include <stdlib.h>

#include "clist.h"
#include "mutex.h"
#include "xtimer.h"
#include "nimble_netif.h"
#include "nimble_netif_conn.h"
#include "net/bluetil/ad.h"
#include "net/bluetil/addr.h"

#include "nimble_scanlist.h"
#include "nimble_scanner.h"

#include "app.h"

static void _on_ble_evt(int handle, nimble_netif_event_t event)
{
    switch (event) {
        case NIMBLE_NETIF_CONNECTED_MASTER: {
            printf("event: handle %i -> CONNECTED as MASTER (", handle);
            bluetil_addr_print(nimble_netif_conn_get(handle)->addr);
            puts(")");
            break;
        }
        case NIMBLE_NETIF_CONNECTED_SLAVE:
            printf("event: handle %i -> CONNECTED as SLAVE (", handle);
            bluetil_addr_print(nimble_netif_conn_get(handle)->addr);
            puts(")");
            break;
        case NIMBLE_NETIF_CLOSED_MASTER:
        case NIMBLE_NETIF_CLOSED_SLAVE:
            printf("event: handle %i -> CONNECTION CLOSED\n", handle);
            break;
        case NIMBLE_NETIF_CONNECT_ABORT:
            printf("event: handle %i -> CONNECTION ABORT\n", handle);
            break;
        case NIMBLE_NETIF_CONN_UPDATED:
        default:
            /* do nothing */
            break;
    }
}

static int _conn_dump(nimble_netif_conn_t *conn, int handle, void *arg)
{
    (void)arg;
    char role = (conn->state & NIMBLE_NETIF_GAP_MASTER) ? 'M' : 'S';

    printf("[%2i] ", handle);
    bluetil_addr_print(conn->addr);
    printf(" (%c) -> ", role);
    bluetil_addr_ipv6_l2ll_print(conn->addr);
    puts("");

    return 0;
}

static int _tmp_dump(nimble_netif_conn_t *conn, int handle, void *arg)
{
    (void)arg;
    printf("[%2i] state: 0x%04x\n", handle, conn->state);
    return 0;
}

static void _conn_list(void)
{
    nimble_netif_conn_foreach(NIMBLE_NETIF_L2CAP_CONNECTED, _conn_dump, NULL);
}

static void _cmd_info(void)
{
    puts("Connection status:");

    unsigned free = nimble_netif_conn_count(NIMBLE_NETIF_UNUSED);
    unsigned active = nimble_netif_conn_count(NIMBLE_NETIF_L2CAP_CONNECTED);

    printf(" Free slots: %u/%u\n", free, NIMBLE_NETIF_CONN_NUMOF);
    printf("Advertising: ");
    if (nimble_netif_conn_get_adv() != NIMBLE_NETIF_CONN_INVALID) {
        puts("yes");
    }
    else {
        puts("no");
    }
    if (active > 0) {
        printf("Connections: %u\n", active);
        _conn_list();
    }
    puts("");

    // REMOVE
    nimble_netif_conn_foreach(0xffff, _tmp_dump, NULL);
}

static void _cmd_adv(const char *name)
{
    int res;
    (void)res;
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    const struct ble_gap_adv_params _adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_LTD,
        .itvl_min = APP_ADV_ITVL,
        .itvl_max = APP_ADV_ITVL,
    };

    /* make sure no advertising is in progress */
    if (nimble_netif_conn_is_adv()) {
        puts("err: advertising already in progress");
        return;
    }

    /* build advertising data */
    res = bluetil_ad_init_with_flags(&ad, buf, BLE_HS_ADV_MAX_SZ,
                                     BLUETIL_AD_FLAGS_DEFAULT);
    assert(res == BLUETIL_AD_OK);
    uint16_t ipss = BLE_SVC_IPSS;
    res = bluetil_ad_add(&ad, BLE_GAP_AD_UUID16_INCOMP, &ipss, sizeof(ipss));
    assert(res == BLUETIL_AD_OK);
    if (name == NULL) {
        name = APP_ADV_NAME_DEFAULT;
    }
    res = bluetil_ad_add(&ad, BLE_GAP_AD_NAME, name, strlen(name));
    if (res != BLUETIL_AD_OK) {
        puts("err: the given name is too long");
        return;
    }

    /* start listening for incoming connections */
    res = nimble_netif_accept(ad.buf, ad.pos, &_adv_params);
    if (res != NIMBLE_NETIF_OK) {
        printf("err: unable to start advertising (%i)\n", res);
    }
    else {
        printf("success: advertising this node as '%s'\n", name);
    }
}

static void _cmd_adv_stop(void)
{
    int res = nimble_netif_accept_stop();
    if (res == NIMBLE_NETIF_OK) {
        puts("canceled advertising");
    }
    else if (res == NIMBLE_NETIF_NOTADV) {
        puts("no advertising in progress");
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
        puts("err: unable to find given entry in scanlist");
        return;
    }

    // TODO: for now we use default parameters
    int res = nimble_netif_connect(&sle->addr, NULL, APP_CONN_TIMEOUT);
    if (res < 0) {
        printf("err: unable to trigger connection sequence (%i)\n", res);
        return;
    }

    printf("initiated connection procedure with ");
    bluetil_addr_print(sle->addr.val);
    puts("");
}

static void _cmd_close(int handle)
{
    int res = nimble_netif_close(handle);
    if (res != NIMBLE_NETIF_OK) {
        puts("err: unable to close connection with given handle");
    }
    else {
        puts("success: connection tear down initiated");
    }
}

static int _ishelp(char *argv)
{
    return memcmp(argv, "help", 4) == 0;
}

void app_ble_init(void)
{
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
        uint32_t duration = APP_SCAN_DUR_DEFAULT;
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
            _conn_list();
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
            _conn_list();
            return 0;
        }
        int handle = atoi(argv[2]);
        _cmd_close(handle);
    }
    else {
        printf("unable to parse the command. Use '%s help' for more help\n",
               argv[0]);
    }

    return 0;
}
