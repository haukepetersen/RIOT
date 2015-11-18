



#ifndef MIA_UDP_H
#define MIA_UDP_H

#include <stdint.h>

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif

void mia_udp_process(void);

int mia_udp_send(uint8_t *ip, uint16_t src, uint16_t dst);
void mia_udp_reply(void);



#ifdef __cplusplus
}
#endif

#endif /* MIA_UDP_H */
