



#ifndef MIA_IP_H
#define MIA_IP_H

#include <stdint.h>
#include <stdlib.h>

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t mia_ip_addr;
extern uint32_t mia_ip_mask;
extern uint32_t mia_ip_bcast;
extern uint32_t mia_ip_gateway;

static inline void mia_ip_set(const int pos, uint32_t ip)
{

}

static inline uint32_t mia_ip_get(const int pos)
{
    return mia_buf[pos];
}

void mia_ip_process(void);

void mia_ip_reply(uint16_t len);
int mia_ip_send(uint8_t *ip, uint8_t proto, uint16_t len);


#ifdef __cplusplus
}
#endif

#endif /* MIA_IP_H */
