




#ifndef MIA_ICMP_H
#define MIA_ICMP_H

#include <stdio.h>

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MIA_ICMP_POS            (34U)
#define MIA_ICMP_TYPE           (MIA_ICMP_POS + 0U)
#define MIA_ICMP_CODE           (MIA_ICMP_POS + 1U)
#define MIA_ICMP_CSUM           (MIA_ICMP_POS + 2U)
#define MIA_ICMP_ECHO_ID        (MIA_ICMP_POS + 4U)
#define MIA_ICMP_ECHO_SEQ       (MIA_ICMP_POS + 6U)
#define MIA_ICMP_ECHO_DATA      (MIA_ICMP_POS + 8U)


void mia_icmp_process(void);


int mia_icmp_ping(uint8_t *addr, mia_cb_t cb);


#ifdef __cplusplus
}
#endif

#endif /* MIA_ICMP_H */
/** @} */
