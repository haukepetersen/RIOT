

#ifndef MIA_ETH_H
#define MIA_ETH_H

#include <stdio.h>

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIA_ETH_HDR_LEN         (14U)

#define MIA_ETH_POS             (0U)
#define MIA_ETH_SRC             (MIA_ETH_POS + 6U)
#define MIA_ETH_DST             (MIA_ETH_POS + 0U)
#define MIA_ETH_TYPE            (MIA_ETH_POS + 12U)


void mia_eth_process(void);

void mia_eth_reply(uint16_t len);

void mia_eth_send(uint8_t *mac, uint16_t ethertype, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* MIA_ETH_H */
/** @} */
