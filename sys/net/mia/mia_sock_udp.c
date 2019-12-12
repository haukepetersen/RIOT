
#include <string.h>

#include "net/af.h"
#include "net/sock/udp.h"
#include "net/iana/portrange.h"

#include "net/mia.h"
#include "net/mia/ip.h"
#include "net/mia/udp.h"

#define ENABLE_DEBUG            (1)
#include "debug.h"

#define EPH_PORT_INC            (17u)

static uint16_t _eph_port = IANA_DYNAMIC_PORTRANGE_MIN;

static uint16_t _eph_get(void)
{
    /* TODO: comply to RFC6056 sec-3.3.3 ?! */
    _eph_port += EPH_PORT_INC;
    if (_eph_port < IANA_DYNAMIC_PORTRANGE_MIN) {
        _eph_port += IANA_DYNAMIC_PORTRANGE_MIN;
    }
    return _eph_port;
}

static void _on_data(mia_bind_t *ep)
{
    sock_udp_t *sock = (sock_udp_t *)ep;

    (void)sock;
}

int sock_udp_create(sock_udp_t *sock, const sock_udp_ep_t *local,
                    const sock_udp_ep_t *remote, uint16_t flags)
{
    assert(sock);

    if (sock->bind.next != NULL) {
        return -EADDRINUSE;
    }
    sock->bind.cb = _on_data;
    if (local) {
        if (local->family != AF_INET) {
            return -EAFNOSUPPORT;
        }
        sock->bind.port = local->port;
    }
    if (remote) {
        if (remote->family != AF_INET) {
            return -EAFNOSUPPORT;
        }
        memcpy(&sock->remote, remote, sizeof(*remote));
    }
    sock->flags = flags;

    (void)local;
    (void)remote;
    (void)flags;
    puts("sock udp create");

    return 0;
}

int sock_udp_get_local(sock_udp_t *sock, sock_udp_ep_t *local)
{
    assert(sock && local);

    local->family = AF_INET;
    memcpy(local->addr.ipv4, mia_ip_addr, MIA_IP_ADDR_LEN);
    local->netif = 0;
    local->port = sock->bind.port;
    return 0;
}

int sock_udp_get_remote(sock_udp_t *sock, sock_udp_ep_t *remote)
{
    assert(sock && remote);

    if (sock->remote.family != AF_INET) {
        return -ENOTCONN;
    }
    memcpy(remote, &sock->remote, sizeof(sock_udp_ep_t));
    return 0;
}

int sock_udp_recv(sock_udp_t *sock, void *data, size_t max_len,
                  uint32_t timeout, sock_udp_ep_t *remote)
{
    (void)sock;
    (void)data;
    (void)max_len;
    (void)timeout;
    (void)remote;

    return 0;
}

ssize_t sock_udp_send(sock_udp_t *sock, const void *data, size_t len,
                      const sock_udp_ep_t *remote)
{
    const sock_udp_ep_t *dst;
    uint16_t src_port;

    /* figure out destination address and port */
    if (sock && sock->remote.family == AF_INET) {
        dst = &sock->remote;
    }
    else if (remote) {
        if (remote->family != AF_INET) {
            return -EAFNOSUPPORT;
        }
        if ((remote->netif != 0) || (remote->port == 0)) {
            return -EINVAL;
        }
        dst = remote;
    }
    else {
        return -ENOTCONN;
    }

    /* and the same for the source port */
    if (sock) {
        if (sock->bind.port == 0) {
            sock->bind.port = _eph_get();
        }
        src_port = sock->bind.port;
    }
    else {
        src_port = _eph_get();
    }

    /* copy data (with checked length) */
    DEBUG("[mia] udp_sock: trying to send %i byte\n", (int)len);
    len = (len > MIA_UDP_MAX_PAYLOAD) ? MIA_UDP_MAX_PAYLOAD : len;

    mia_lock();
    memcpy(mia_ptr(MIA_APP_POS), data, len);
    mia_ston(MIA_UDP_LEN, MIA_UDP_HDR_LEN + len);
    int res = mia_udp_send(dst->addr.ipv4, src_port, dst->port);
    mia_unlock();

    return (res < 0) ? -EHOSTUNREACH : (ssize_t)len;
}
