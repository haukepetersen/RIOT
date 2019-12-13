


#ifndef MIA_DNS_H
#define MIA_DNS_H

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * how we do it:
 * - call mia_dns_request(), pass request ctx

#define MIA_DNS_CLI_PORT        (53726U)     /* TODO: use ephemeral port */
#define MIA_DNS_SRV_PORT        (53U)

extern mia_bind_t mia_dns_ep;

typedef struct {
    mia_bind_t bind;
    // xtimer_t timeout;    /* TODO: timeout */
} mia_dns_req_t;

void mia_dns_request(mia_dns_req_t *req);

void mia_dns_process(mia_bind_t *ep);

#ifdef __cplusplus
}
#endif

#endif /* MIA_DHCP_H */
/** @} */
