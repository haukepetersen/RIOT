


#ifndef MIA_COAP_H
#define MIA_COAP_H

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIA_COAP_PORT       (5683U)

extern mia_bind_t mia_coap_ep;

typedef uint16_t(*mia_coap_cb)(void);

typedef struct {
    const char *ep;
    mia_coap_cb cb;
} mia_coap_ep_t;

void mia_coap_process(void);


#ifdef __cplusplus
}
#endif

#endif /* MIA_COAP_H */
