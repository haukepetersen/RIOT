


#ifndef MIA_ARP_CACHE_H
#define MIA_ARP_CACHE_H

#include <stdio.h>

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif


uint8_t *mia_arp_cache_lookup(uint8_t *ip);

void mia_arp_cache_insert(uint8_t *ip, uint8_t *mac);

#endif /* MIA_ARP_CACHE_H */
/** @} */
