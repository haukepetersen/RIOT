






#ifndef MIA_ARP_H
#define MIA_ARP_H

#include <stdio.h>

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif





void mia_arp_process(void);

void mia_arp_request(uint8_t *ip);


#endif /* MIA_ARP_H */
/** @} */
