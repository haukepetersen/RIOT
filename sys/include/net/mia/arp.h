






#ifndef MIA_ARP_H
#define MIA_ARP_H

#include <stdio.h>

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIA_ARP_POS             (14U)
#define MIA_ARP_HTYPE           (MIA_ARP_POS + 0U)
#define MIA_ARP_PTYPE           (MIA_ARP_POS + 2U)
#define MIA_ARP_HLEN            (MIA_ARP_POS + 4U)
#define MIA_ARP_PLEN            (MIA_ARP_POS + 5U)
#define MIA_ARP_OPER            (MIA_ARP_POS + 6U)
#define MIA_ARP_SHA             (MIA_ARP_POS + 8U)
#define MIA_ARP_SPA             (MIA_ARP_POS + 14U)
#define MIA_ARP_THA             (MIA_ARP_POS + 18U)
#define MIA_ARP_TPA             (MIA_ARP_POS + 24U)




void mia_arp_process(void);

void mia_arp_request(uint8_t *ip);


#endif /* MIA_ARP_H */
/** @} */
