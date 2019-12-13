

#include <stdint.h>
#include <stdlib.h>

#include "net/protnum.h"

#include "net/mia.h"
#include "net/mia/ip.h"
#include "net/mia/udp.h"
#include "net/iana/portrange.h"

#define ENABLE_DEBUG            (0)
#include "debug.h"

#define EPH_PORT_INC            (17u)

static uint16_t _eph_port = IANA_DYNAMIC_PORTRANGE_MIN;

static mia_bind_t *endpoints = NULL;

static uint16_t _eph_get(void)
{
    /* TODO: comply to RFC6056 sec-3.3.3 ?! */
    _eph_port += EPH_PORT_INC;
    if (_eph_port < IANA_DYNAMIC_PORTRANGE_MIN) {
        _eph_port += IANA_DYNAMIC_PORTRANGE_MIN;
    }
    return _eph_port;
}

int mia_udp_bind(mia_bind_t *ctx)
{
    for (mia_bind_t *ep = endpoints; ep; ep = ep->next) {
        if (ep->port == ctx->port) {
            DEBUG("[mia] udp: bind error: port %i in use\n", (int)ctx->port);
            return MIA_ERR_NO_PORT;
        }
    }

    DEBUG("[mia] udp: adding ep, port %i\n", (int)ctx->port);
    ctx->next = endpoints;
    endpoints = ctx;

    return MIA_OK;
}

int mia_udp_bind_eph(mia_bind_t *ctx)
{
    int res = MIA_ERR_NO_PORT;
    while (res == MIA_ERR_NO_PORT) {
        ctx->port = _eph_get();
        res = mia_udp_bind(ctx);
    }
    return res;
}

int mia_udp_unbind(mia_bind_t *ep)
{
    mia_bind_t *b = endpoints;

    if (b == ep) {
        endpoints = b->next;
        return MIA_OK;
    }

    while (b->next) {
        if (b->next == ep) {
            b->next = b->next->next;
            return MIA_OK;
        }
    }

    return MIA_ERR_NO_PORT;
}

void mia_udp_process(void)
{
    for (mia_bind_t *ep = endpoints; ep; ep = ep->next) {
        if (ep->port == mia_ntos(MIA_UDP_DST)) {
            DEBUG("[mia] udp: relaying request to port %i\n", (int)ep->port);
            ep->cb(ep);
            return;
        }
    }

    DEBUG("[mia] udp: can't handle packet to port %i\n", (int)mia_ntos(MIA_UDP_DST));
    // mia_icmp_unreachable(ICMP_CODE_PORT_UNREACHABLE);
}

void mia_udp_reply(void)
{
    uint8_t tmp[2];

    /* switch UDP ports, set csum to zero and adjust length */
    memcpy(tmp, mia_ptr(MIA_UDP_SRC), 2);
    memcpy(mia_ptr(MIA_UDP_SRC), mia_ptr(MIA_UDP_DST), 2);
    memcpy(mia_ptr(MIA_UDP_DST), tmp, 2);
    memset(mia_ptr(MIA_UDP_CSUM), 0, 2);
    mia_ip_reply(mia_ntos(MIA_UDP_LEN));
}

int mia_udp_send(const uint8_t *ip, uint16_t src, uint16_t dst)
{
    mia_ston(MIA_UDP_SRC, src);
    mia_ston(MIA_UDP_DST, dst);
    memset(mia_ptr(MIA_UDP_CSUM), 0, 2);
    return mia_ip_send(ip, PROTNUM_UDP, mia_ntos(MIA_UDP_LEN));
}
