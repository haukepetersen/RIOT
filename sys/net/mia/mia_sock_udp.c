
// #include <string.h>

// #include "net/sock/udp.h"

// #include "net/mia.h"
// #include "net/mia/udp.h"

// #define ENABLE_DEBUG            (1)
// #include "debug.h"

// uint16_t random_src_port = 34568;


// int sock_udp_create(sock_udp_t *sock, const sock_udp_ep_t *local,
//                     const sock_udp_ep_t *remote, uint16_t flags)
// {
//     puts("sock udp create");
// }

// ssize_t sock_udp_send(sock_udp_t *sock, const void *data, size_t len,
//                       const sock_udp_ep_t *remote)
// {
//     mia_lock();

//     (void)sock;

//     /* copy data (with checked length) */
//     DEBUG("[mia] udp_sock: trying to send %i byte\n", (int)len);
//     len = (len > MIA_UDP_MAX_PAYLOAD) ? MIA_UDP_MAX_PAYLOAD : len;
//     memcpy(mia_ptr(MIA_APP_POS), data, len);

//     mia_ston(MIA_UDP_LEN, MIA_UDP_HDR_LEN + len);
//     mia_udp_send(remote->addr.ipv4, random_src_port, remote->port);

//     mia_unlock();

//     return len;
// }
