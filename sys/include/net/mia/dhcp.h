


#ifndef MIA_DHCP_H
#define MIA_DHCP_H

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DHCP_CLI_PORT           (68U)
#define DHCP_SRV_PORT           (67U)

void mia_dhcp_request(void);

void mia_dhcp_process(void);

#ifdef __cplusplus
}
#endif

#endif /* MIA_DHCP_H */
/** @} */
