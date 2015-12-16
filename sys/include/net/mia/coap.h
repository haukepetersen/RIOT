


#ifndef MIA_COAP_H
#define MIA_COAP_H

#include "net/mia.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*mia_coap_cb)(void);

typedef struct {
    uint32_t ep;
    mia_cb_t get;
    mia_cb_t post;
} mia_coap_ep_t;

void mia_coap_process(void);


#ifdef __cplusplus
}
#endif

#endif /* MIA_COAP_H */
