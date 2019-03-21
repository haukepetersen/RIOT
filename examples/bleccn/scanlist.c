
#include <limits.h>

#include "adproc.h"
#include "xtimer.h"
#include "scanlist.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

static scanlist_t _mem[SCANLIST_SIZE];

static clist_node_t _pool;
static clist_node_t _list;


static void _print_addr(const ble_addr_t *addr)
{
    printf("%02x", (int)addr->val[5]);
    for (int i = 4; i >= 0; i--) {
        printf(":%02x", addr->val[i]);
    }
    switch (addr->type) {
        case BLE_ADDR_PUBLIC:       printf(" (PUBLIC) ");   break;
        case BLE_ADDR_RANDOM:       printf(" (RANDOM) ");   break;
        case BLE_ADDR_PUBLIC_ID:    printf(" (PUB_ID) ");   break;
        case BLE_ADDR_RANDOM_ID:    printf(" (RAND_ID)");  break;
        default:                    printf(" (UNKNOWN)");  break;
    }
}

static int _printer(clist_node_t *node, void *arg)
{
    (void)arg;
    scanlist_t *e = (scanlist_t *)node;

    assert(e);

    /* try to find a device name */
    char name[(BLE_ADV_PDU_LEN + 1)] = { 0 };
    int res = adproc_get_str(BLE_GAP_AD_NAME, name, e->ad, e->ad_len);
    if (res != ADPROC_OK) {
        res = adproc_get_str(BLE_GAP_AD_NAME_SHORT, name, e->ad, e->ad_len);
    }
    if (res != ADPROC_OK) {
        strcpy(name, "undefined");
    }

    printf("- ");
    _print_addr(&e->addr);
    unsigned adv_int = ((e->last_update - e->first_update) / e->adv_msg_cnt);
    printf(" \"%s\", adv_msg_cnt: %u, adv_int: %uus\n",
           name, (unsigned)e->adv_msg_cnt, adv_int);
    return 0;
}

static int _finder(clist_node_t *node, void *arg)
{
    const ble_addr_t *addr = (const ble_addr_t *)arg;
    scanlist_t *e = (scanlist_t *)node;
    if (ble_addr_cmp(&e->addr, addr) == 0) {
        return 1;
    }
    return 0;
}

void scanlist_init(void)
{
    for (unsigned i = 0; i < (sizeof(_mem) / sizeof(_mem[0])); i++) {
        clist_rpush(&_pool, &_mem[i].node);
    }
}

scanlist_t *_find_and_remove(const ble_addr_t *addr)
{
    clist_node_t * e = clist_foreach(&_list, _finder, (void *)addr);
    if (e) {
        clist_remove(&_list, e);
    }
    return (scanlist_t *)e;
}

void scanlist_update(const ble_addr_t *addr, const uint8_t *ad, size_t len)
{
    assert(addr);
    assert(len <= BLE_ADV_PDU_LEN);

    uint32_t now = xtimer_now_usec();
    scanlist_t *e = _find_and_remove(addr);

    if (!e) {
        e = (scanlist_t *)clist_lpop(&_pool);
        if (!e) {
            /* no space available, dropping newly discovered node */
            return;
        }
        memcpy(&e->addr, addr, sizeof(ble_addr_t));
        if (ad) {
            memcpy(e->ad, ad, len);
        }
        e->ad_len = len;
        e->first_update = now;
        e->adv_msg_cnt = 1;
    }
    else {
        e->adv_msg_cnt++;
    }

    e->last_update = now;
    clist_rpush(&_list, (clist_node_t *)e);
}

void scanlist_clear(void)
{
    clist_node_t *node = clist_lpop(&_list);
    while (node) {
        clist_rpush(&_pool, node);
        node = clist_lpop(&_list);
    }
}

void scanlist_print(void)
{
    clist_foreach(&_list, _printer, NULL);
}
