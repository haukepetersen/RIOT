

#include "net/ethernet.h"

#include "net/mia/arp_cache.h"

typedef struct {
    uint8_t ip[MIA_IP_ADDR_LEN];
    uint8_t mac[ETHERNET_ADDR_LEN];
} arp_entry_t;

static arp_entry_t arp_cache[MIA_ARP_CACHE_SIZE];
static int arp_cache_next = 0;


uint8_t *mia_arp_cache_lookup(uint8_t *ip)
{
    for (int i = 0; i < MIA_ARP_CACHE_SIZE; i++) {
        if (memcmp(ip, arp_cache[i].ip, MIA_IP_ADDR_LEN) == 0) {
            return arp_cache[i].mac;
        }
    }
    return NULL;
}

void mia_arp_cache_insert(uint8_t *ip, uint8_t *mac)
{
    for (int i = 0; i < MIA_ARP_CACHE_SIZE; i++) {
        if (memcmp(ip, arp_cache[i].ip, MIA_IP_ADDR_LEN) == 0) {
            memcpy(arp_cache[i].mac, mac, ETHERNET_ADDR_LEN);
            return;
        }
    }
    memcpy(arp_cache[arp_cache_next].ip, ip, MIA_IP_ADDR_LEN);
    memcpy(arp_cache[arp_cache_next].mac, mac, ETHERNET_ADDR_LEN);
    ++arp_cache_next;
}
