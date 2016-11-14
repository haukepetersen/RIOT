



#ifndef MIA_UDP_H
#define MIA_UDP_H

#include <stdint.h>

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIA_UDP_POS             (34U)
#define MIA_UDP_SRC             (MIA_UDP_POS + 0U)
#define MIA_UDP_DST             (MIA_UDP_POS + 2U)
#define MIA_UDP_LEN             (MIA_UDP_POS + 4U)
#define MIA_UDP_CSUM            (MIA_UDP_POS + 6U)


int mia_udp_bind(mia_bind_t *ep);
int mia_udp_unbind(mia_bind_t *ep);

void mia_udp_process(void);

int mia_udp_send(uint8_t *ip, uint16_t src, uint16_t dst);
void mia_udp_reply(void);



#ifdef __cplusplus
}
#endif

#endif /* MIA_UDP_H */
