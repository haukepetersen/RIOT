




#ifndef MIA_ICMP_H
#define MIA_ICMP_H

#include <stdio.h>

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif



void mia_icmp_process(void);


int mia_icmp_ping(uint8_t *addr, mia_cb_t cb);


#ifdef __cplusplus
}
#endif

#endif /* MIA_ICMP_H */
/** @} */
