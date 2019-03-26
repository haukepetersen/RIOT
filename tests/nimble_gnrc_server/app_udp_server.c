

#include "thread.h"
#include "net/sock/udp.h"
#include "net/gnrc/rpl.h"
#include "net/gnrc/netif.h"
#include "net/netif.h"
#include "net/ipv6/addr.h"

#include "nimble_gnrc_test_conf.h"
#include "app_udp_server.h"

/* UDP server state */
static char _udp_stack[THREAD_STACKSIZE_DEFAULT];
static uint32_t _udp_buf[UDP_BUFSIZE / 4];

static void *_udp_server(void *arg)
{
    (void)arg;
    int res;
    (void)res;

    sock_udp_t sock;
    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = APP_PORT;
    res = sock_udp_create(&sock, &local, NULL, 0);
    assert(res == 0);

    while (1) {
        sock_udp_ep_t remote;
        ssize_t len = sock_udp_recv(&sock, _udp_buf, sizeof(_udp_buf),
                                    SOCK_NO_TIMEOUT, &remote);
        assert(len > 0);
        assert((size_t)len <= sizeof(_udp_buf));

        /* dump the meta data of the received packet */
        printf("# Received: len %5i, seq %5u\n", len, (unsigned)_udp_buf[POS_SEQ]);

        /* echo the received data */
        ssize_t sent = sock_udp_send(&sock, _udp_buf, (size_t)len, &remote);
        assert(sent == len);
    }

    /* never reached (at least in theory) */
    return NULL;
}

void app_udp_server_run(void)
{
    int res;
    (void)res;
    ipv6_addr_t addr;
    ipv6_addr_t *addr_test;
    (void)addr_test;

    /* manually assign a global address and initialize the RPL root */
    addr_test = ipv6_addr_from_str(&addr, APP_ADDR);
    assert(addr_test != NULL);
    netif_t iface = netif_iter(NETIF_INVALID);
    assert(iface != NETIF_INVALID);
    res = gnrc_netapi_set(iface, NETOPT_IPV6_ADDR, 0, &addr, sizeof(addr));

    gnrc_rpl_instance_t *inst = gnrc_rpl_root_init(RPL_INSTANCE, &addr,
                                                   false, false);
    assert(inst != NULL);

    thread_create(_udp_stack, sizeof(_udp_stack), UDP_SERVER_PRIO,
                  THREAD_CREATE_STACKTEST, _udp_server, NULL, "udp_echo");
}
