

#include "net/ethernet.h"
#include "net/mia/arp_cache.h"

typedef struct {
    uint8_t ip[MIA_IP_ADDR_LEN];
    uint8_t mac[ETHERNET_ADDR_LEN];
} arp_entry_t;

static arp_entry_t _arp_cache[MIA_ARP_CACHE_SIZE];
static unsigned _arp_cache_next = 0;

uint8_t *mia_arp_cache_lookup(uint8_t *ip)
{
    for (unsigned int i = 0; i < MIA_ARP_CACHE_SIZE; i++) {
        if (memcmp(ip, _arp_cache[i].ip, MIA_IP_ADDR_LEN) == 0) {
            return _arp_cache[i].mac;
        }
    }
    return NULL;
}

void mia_arp_cache_insert(uint8_t *ip, uint8_t *mac)
{
    for (unsigned int i = 0; i < MIA_ARP_CACHE_SIZE; i++) {
        if (memcmp(ip, _arp_cache[i].ip, MIA_IP_ADDR_LEN) == 0) {
            memcpy(_arp_cache[i].mac, mac, ETHERNET_ADDR_LEN);
            return;
        }
    }
    memcpy(_arp_cache[_arp_cache_next].ip, ip, MIA_IP_ADDR_LEN);
    memcpy(_arp_cache[_arp_cache_next].mac, mac, ETHERNET_ADDR_LEN);
    if (++_arp_cache_next >= MIA_ARP_CACHE_SIZE) {
        _arp_cache_next = 0;
    }
}
