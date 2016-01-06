



#ifndef MIA_IP_H
#define MIA_IP_H

#include <stdint.h>
#include <stdlib.h>

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIA_IP_ADDR             {10, 10, 10, 23}
#define MIA_IP_MASK             (3U)
#define MIA_IP_BCAST            {192, 168, 2, 255}
#define MIA_IP_GATEWAY          {192, 168, 2, 1}

/**
 * @brief   Default IP address configuration
 *
 * Do not override if you are using DHCP
 *
 * @{
 */
#ifndef MIA_IP_ADDR
#define MIA_IP_ADDR             {0, 0, 0, 0}
#endif
#ifndef MIA_IP_MASK
#define MIA_IP_MASK             (0U)
#endif
#ifndef MIA_IP_BCAST
#define MIA_IP_BCAST            {0xff, 0xff, 0xff, 0xff}
#endif
#ifndef MIA_IP_GATEWAY
#define MIA_IP_GATEWAY          {0xff, 0xff, 0xff, 0xff}
#endif
/** @} */


extern uint8_t mia_ip_addr[];
extern uint8_t mia_ip_mask;
extern uint8_t mia_ip_bcast[];
extern uint8_t mia_ip_gateway[];


void mia_ip_process(void);

void mia_ip_reply(uint16_t len);
int mia_ip_send(uint8_t *ip, uint8_t proto, uint16_t len);


#ifdef __cplusplus
}
#endif

#endif /* MIA_IP_H */
