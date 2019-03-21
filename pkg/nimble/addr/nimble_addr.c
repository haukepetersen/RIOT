
#include "nimble_addr.h"

void nimble_addr_print(const ble_addr_t *addr)
{
    char tmp[NIMBLE_ADDR_STR_MAXLEN];
    nimble_addr_sprint(tmp, addr);
    printf("%s", tmp);
}

void nimble_addr_sprint(char *out, const ble_addr_t *addr)
{
    out += sprintf(out, "%02x", (int)addr->val[5]);
    for (int i = 4; i >= 0; i--) {
        out += sprintf(out, ":%02x", (int)addr->val[i]);
    }
    switch (addr->type) {
        case BLE_ADDR_PUBLIC:       sprintf(out, " (public) "); break;
        case BLE_ADDR_RANDOM:       sprintf(out, " (random) "); break;
        case BLE_ADDR_PUBLIC_ID:    sprintf(out, " (id_pub) "); break;
        case BLE_ADDR_RANDOM_ID:    sprintf(out, " (id_rand)"); break;
        default:                    sprintf(out, " (unknown)"); break;
    }
}
