
#ifndef SOCK_TYPES_H_
#define SOCK_TYPES_H_

#include "mia.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _sock_tl_ep {
    mia_bind_t bind;
    sock_udp_ep_t remote;
    uint16_t flags;
};


#ifdef __cplusplus
}
#endif

#endif /* SOCK_TYPES_H_ */
