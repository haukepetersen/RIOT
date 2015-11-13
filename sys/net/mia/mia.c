
#include <errno.h>

#include "msg.h"
#include "mutex.h"
#include "thread.h"
#include "xtimer.h"
#include "net/netopt.h"
#include "net/protnum.h"
#include "net/ethernet.h"
#include "net/ethertype.h"
#include "net/inet_csum.h"

#include "net/mia.h"

#define ENABLE_DEBUG            (0)
#include "debug.h"

#define DHCP_POS                (42U)
#define DHCP_OP                 (DHCP_POS + 0U)
#define DHCP_HTYPE              (DHCP_POS + 1U)
#define DHCP_HLEN               (DHCP_POS + 2U)
#define DHCP_HOPS               (DHCP_POS + 3U)
#define DHCP_XID                (DHCP_POS + 4U)
#define DHCP_SECS               (DHCP_POS + 8U)
#define DHCP_FLAGS              (DHCP_POS + 10U)
#define DHCP_CIADDR             (DHCP_POS + 12U)


#define ARP_OPER_REQ            (1U)
#define ARP_OPER_RPLY           (2U)


#define ICMP_TYPE_ECHO_REQ      (8U)
#define ICMP_TYPE_ECHO_RPLY     (0U)
#define ICMP_ECHO_ID            (0xdeed)

#define IP_ADDR_LEN             (4U)
#define IP_VER_VAL              (0x45)
#define IP_DEFAULT_TTL          (64U)


typedef struct {
    uint8_t ip[IP_ADDR_LEN];
    uint8_t mac[ETHERNET_ADDR_LEN];
} arp_entry_t;

static const uint8_t arp_head[] = {0x00, 0x01, 0x08, 0x00, 0x06, 0x04};
static const uint8_t bcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


static mutex_t lock;

static msg_t msg_queue[MIA_MSG_QUEUESIZE];
static kernel_pid_t mia_pid;

static netdev2_t *dev;
static uint8_t my_mac[ETHERNET_ADDR_LEN];


uint8_t mia_buf[MIA_BUFSIZE];
static size_t buf_len;


static arp_entry_t arp_cache[MIA_ARP_CACHE_SIZE];
static int arp_cache_next = 0;


static uint8_t mia_ip_addr[] = {0, 0, 0, 0};
static uint8_t mia_ip_mask = 4;
static uint8_t mia_ip_gateway[] = {0, 0, 0, 0};

static mia_cb_t ping_cb = NULL;
static uint16_t ping_seq = 0;

static mia_bind_t *udp_bindings = NULL;


#if ENABLE_DEBUG
static void dbg_eth_dump(void)
{
    DEBUG("[mia] eth: dst ");
    mia_dump_mac(MIA_ETH_DST);
    DEBUG(", src ");
    mia_dump_mac(MIA_ETH_SRC);
    DEBUG(", type: 0x%02x%02x\n",
          mia_buf[MIA_ETH_TYPE], mia_buf[MIA_ETH_TYPE + 1]);
}

static void dbg_arp_dump(void)
{
    DEBUG("[mia] arp: OPER: %i\n", (int)mia_ntos(MIA_ARP_OPER));
    DEBUG("      arp: SHA ");
    mia_dump_mac(MIA_ARP_SHA);
    DEBUG(", SPA ");
    mia_dump_ip(MIA_ARP_SPA);
    DEBUG("\n           THA ");
    mia_dump_mac(MIA_ARP_THA);
    DEBUG(", TPA ");
    mia_dump_ip(MIA_ARP_TPA);
    DEBUG("\n");
}

static void dbg_ip_dump(void)
{
    DEBUG("[mia] ip:  SRC ");
    mia_dump_ip(MIA_IP_SRC);
    DEBUG(", DST ");
    mia_dump_ip(MIA_IP_DST);
    DEBUG(", PROTO: %i, LEN: %i\n",
          mia_buf[MIA_IP_PROTO], (int)mia_ntos(MIA_IP_LEN));
}

static void dbg_icmp_dump(void)
{
    DEBUG("[mia] icmp: TYPE:%i , CODE:%i\n",
          mia_buf[MIA_ICMP_TYPE], mia_buf[MIA_ICMP_CODE]);
}

static void dbg_udp_dump(void)
{
    DEBUG("[mia] udp:  SRC:%i, DST:%i, LEN:%i\n",
          mia_ntos(MIA_UDP_SRC), mia_ntos(MIA_UDP_DST), mia_ntos(MIA_UDP_LEN));
}

#else
#define dbg_eth_dump(x)
#define dbg_arp_dump(x)
#define dbg_ip_dump(x)
#define dbg_icmp_dump(x)
#define dbg_udp_dump(x)
#endif


static uint8_t *arp_cache_lookup(uint8_t *ip)
{
    for (int i = 0; i < MIA_ARP_CACHE_SIZE; i++) {
        if (memcmp(ip, arp_cache[i].ip, IP_ADDR_LEN) == 0) {
            return arp_cache[i].mac;
        }
    }
    return NULL;
}

static void arp_cache_insert(uint8_t *ip, uint8_t *mac)
{
    for (int i = 0; i < MIA_ARP_CACHE_SIZE; i++) {
        if (memcmp(ip, arp_cache[i].ip, IP_ADDR_LEN) == 0) {
            memcpy(arp_cache[i].mac, mac, ETHERNET_ADDR_LEN);
            return;
        }
    }
    memcpy(arp_cache[arp_cache_next].ip, ip, IP_ADDR_LEN);
    memcpy(arp_cache[arp_cache_next].mac, mac, ETHERNET_ADDR_LEN);
    ++arp_cache_next;

#if ENABLE_DEBUG
    DEBUG("ARP CACHE:\n");
    for (int i = 0; i < MIA_ARP_CACHE_SIZE; i++) {
        uint8_t *p = arp_cache[i].ip;
        uint8_t *m = arp_cache[i].mac;
        printf("-%i- %i.%i.%i.%i <--> ",
              i, (int)p[0], (int)p[1], (int)p[2], (int)p[3]);
        printf("0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n",
               m[0], m[1], m[2], m[3], m[4], m[5]);
    }
#endif
}

static void flush(void)
{
    struct iovec data;
    /* set iovec */
    data.iov_base = (void *)mia_buf;
    data.iov_len = buf_len;
    dev->driver->send(dev, &data, 1);
}

static void eth_reply(void)
{
    memcpy(mia_ptr(MIA_ETH_DST), mia_ptr(MIA_ETH_SRC), ETHERNET_ADDR_LEN);
    memcpy(mia_ptr(MIA_ETH_SRC), my_mac, ETHERNET_ADDR_LEN);
    flush();
}

static void eth_send(uint8_t *mac, uint16_t ethertype)
{
    memcpy(mia_ptr(MIA_ETH_DST), mac, ETHERNET_ADDR_LEN);
    memcpy(mia_ptr(MIA_ETH_SRC), my_mac, ETHERNET_ADDR_LEN);
    mia_ston(MIA_ETH_TYPE, ethertype);
    flush();
}

static void arp_request(uint8_t *ip)
{
    /* put ARP header on top */
    memcpy(mia_ptr(MIA_ARP_POS), arp_head, 6);
    mia_ston(MIA_ARP_OPER, ARP_OPER_REQ);
    memcpy(mia_ptr(MIA_ARP_SHA), my_mac, ETHERNET_ADDR_LEN);
    memcpy(mia_ptr(MIA_ARP_SPA), mia_ip_addr, IP_ADDR_LEN);
    memset(mia_ptr(MIA_ARP_THA), 0, ETHERNET_ADDR_LEN);
    memcpy(mia_ptr(MIA_ARP_TPA), ip, IP_ADDR_LEN);

    dbg_arp_dump();

    /* send packet */
    buf_len = MIA_ARP_HDR_LEN + MIA_ETH_HDR_LEN + 1;
    eth_send((uint8_t *)bcast, ETHERTYPE_ARP);
}

static void ip_csum(uint16_t len)
{
    buf_len = MIA_ETH_HDR_LEN + MIA_IP_HDR_LEN + len;
    mia_ston(MIA_IP_LEN, len + MIA_IP_HDR_LEN);
    memset(mia_ptr(MIA_IP_CSUM), 0, 2);
    mia_ston(MIA_IP_CSUM, ~inet_csum(0, mia_ptr(MIA_IP_POS), MIA_IP_HDR_LEN));
}

static void ip_reply(uint16_t len)
{
    memcpy(mia_ptr(MIA_IP_DST), mia_ptr(MIA_IP_SRC), IP_ADDR_LEN);
    memcpy(mia_ptr(MIA_IP_SRC), mia_ip_addr, IP_ADDR_LEN);
    ip_csum(len);
    eth_reply();
}

static int ip_send(uint8_t *ip, uint8_t proto, uint16_t len)
{
    uint8_t *ta, *mac;

    if (memcmp(ip, bcast, IP_ADDR_LEN) == 0) {
        mac = (uint8_t *)bcast;
    }
    else {
        ta = (memcmp(ip, mia_ip_addr, mia_ip_mask) == 0) ? ip : mia_ip_gateway;
        mac = arp_cache_lookup(ta);
        if (mac == NULL) {
            arp_request(ta);
            DEBUG("[mia] ping: host not in ARP cache, requesting now\n");
            return MIA_ERR_NO_ROUTE;
        }
    }

    /* fill the IPv4 header */
    mia_buf[MIA_IP_VER] = IP_VER_VAL;
    memset(mia_ptr(MIA_IP_VER + 1), 0, 7);
    mia_buf[MIA_IP_TTL] = IP_DEFAULT_TTL;
    mia_buf[MIA_IP_PROTO] = proto;
    memcpy(mia_ptr(MIA_IP_DST), ip, IP_ADDR_LEN);
    memcpy(mia_ptr(MIA_IP_SRC), mia_ip_addr, IP_ADDR_LEN);
    /* and finish by calculating the checksum */
    ip_csum(len);
    /* finally send out the packet */
    eth_send(mac, ETHERTYPE_IPV4);
    return 0;
}


static void udp_process(void)
{
    dbg_udp_dump();

    if (!udp_bindings) {
        return;
    }

    int i = 0;
    while (udp_bindings && udp_bindings[i].port != 0) {
        if (udp_bindings[i].port == mia_ntos(MIA_UDP_DST)) {
            udp_bindings[i].cb();
            return;
        }
        ++i;
    }
}

static void icmp_process(void)
{
    dbg_icmp_dump();

    /* we only react to echo requests */
    if (mia_buf[MIA_ICMP_TYPE] == ICMP_TYPE_ECHO_REQ) {
        /* set type to reply and re-calculate checksum */
        mia_buf[MIA_ICMP_TYPE] = ICMP_TYPE_ECHO_RPLY;
        mia_ston(MIA_ICMP_CSUM, 0);
        mia_ston(MIA_ICMP_CSUM, ~inet_csum(0, mia_ptr(MIA_ICMP_POS),
                                       mia_ntos(MIA_IP_LEN) - MIA_IP_HDR_LEN));
        ip_reply(mia_ntos(MIA_IP_LEN));
    }
    else {
        if (ping_cb) {
            ping_cb();
        }
    }
}

static void ip_process(void)
{
    dbg_ip_dump();

    /* packet for me? */
    if (!(memcmp(mia_ptr(MIA_IP_DST), mia_ip_addr, IP_ADDR_LEN) == 0 ||
          memcmp(mia_ptr(MIA_IP_DST), bcast, IP_ADDR_LEN) == 0)) {
        DEBUG("[mia] ip:  got packet that was not for me\n");
        return;
    }

    switch (mia_buf[MIA_IP_PROTO]) {
        case PROTNUM_ICMP:
            icmp_process();
            break;
        case PROTNUM_UDP:
            udp_process();
            break;
    }
}

static void arp_process(void)
{
    /* check hardware type and protocol type as well as HLEN and PLEN*/
    if (memcmp(mia_ptr(MIA_ARP_POS), arp_head, 6) != 0) {
        DEBUG("[mia] arp: error: packet is not IPv4 over Ethernet\n");
        return;
    }

    dbg_arp_dump();

    /* insert new information into ARP cache */
    arp_cache_insert(mia_ptr(MIA_ARP_SPA), mia_ptr(MIA_ARP_SHA));

    if (mia_ntos(MIA_ARP_OPER) == ARP_OPER_REQ) {
        /* was the request for my IP address? */
        if (memcmp(mia_ptr(MIA_ARP_TPA), mia_ip_addr, IP_ADDR_LEN) == 0) {
            mia_ston(MIA_ARP_OPER, ARP_OPER_RPLY);
            memcpy(mia_ptr(MIA_ARP_THA), mia_ptr(MIA_ARP_SHA),
                   ETHERNET_ADDR_LEN + IP_ADDR_LEN);
            memcpy(mia_ptr(MIA_ARP_SHA), my_mac, ETHERNET_ADDR_LEN);
            memcpy(mia_ptr(MIA_ARP_SPA), mia_ip_addr, IP_ADDR_LEN);
            DEBUG("[mia] arp: request for me, answering it\n");
            eth_reply();
        }
    }
}

static void eth_process(void)
{
    /* only allow packets addressed to me or broadcast packets */
    if (!(memcmp(mia_ptr(MIA_ETH_DST), my_mac, ETHERNET_ADDR_LEN) == 0 ||
          memcmp(mia_ptr(MIA_ETH_DST), bcast, ETHERNET_ADDR_LEN) == 0)) {
        return;
    }

    dbg_eth_dump();

    /* IPv4 or ARP */
    switch (mia_ntos(MIA_ETH_TYPE)) {
        case ETHERTYPE_ARP:
            arp_process();
            break;
        case ETHERTYPE_IPV4:
            ip_process();
            break;
        default:
            DEBUG("[mia] got packet that we can not handle\n");
            break;
    }
}

static void on_evt(netdev2_t *dev, netdev2_event_t evt, void *arg)
{
    (void)arg;
    msg_t msg;

    switch (evt) {
        case NETDEV2_EVENT_ISR:
            msg.type = MIA_MSG_ISR;
            msg_send(&msg, mia_pid);
            break;
        case NETDEV2_EVENT_RX_COMPLETE:
            mutex_lock(&lock);
            DEBUG("[mia] ------ RX complete ------\n");
            buf_len = dev->driver->recv(dev, (char *)mia_buf, MIA_BUFSIZE);
            eth_process();
            mutex_unlock(&lock);
            break;
        default:
            /* not interested in anything else for now */
            break;
    }
}

int mia_run(netdev2_t *netdev)
{
    uint16_t type;
    msg_t msg;
    dev = netdev;

    /* device must be given */
    if (dev == NULL) {
        return -ENODEV;
    }
    /* so far we can only handle Ethernet */
    dev->driver->get(dev, NETOPT_DEVICE_TYPE, (void *)&type, 2);
    if (type != NETDEV2_TYPE_ETHERNET) {
        return -ENOTSUP;
    }

    /* init mutex */
    mutex_init(&lock);
    /* setup the message queue and remember the PID */
    msg_init_queue(msg_queue, MIA_MSG_QUEUESIZE);
    mia_pid = thread_getpid();
    /* bootstrap the network device */
    dev->event_callback = on_evt;
    dev->isr_arg = NULL;
    dev->driver->init(dev);

    /* get the MAC address from the device driver */
    dev->driver->get(dev, NETOPT_ADDRESS, &my_mac, ETHERNET_ADDR_LEN);

    DEBUG("[mia] initialization seems ok, running now\n");
    while (1) {
        msg_receive(&msg);
        if (msg.type == MIA_MSG_ISR) {
            dev->driver->isr(dev);
        }
    }

    /* we should not end up here... */
    return 0;
}


void mia_ip_config(uint8_t *addr, uint8_t mask, uint8_t *gw)
{
    mutex_lock(&lock);
    memcpy(mia_ip_addr, addr, IP_ADDR_LEN);
    memcpy(mia_ip_gateway, gw, IP_ADDR_LEN);
    mia_ip_mask = mask;
    mutex_unlock(&lock);
}

int mia_ping(uint8_t *addr, mia_cb_t cb)
{
    int res = 0;
    uint32_t now = xtimer_now();

    mutex_lock(&lock);

    ping_cb = cb;
    mia_buf[MIA_ICMP_TYPE] = ICMP_TYPE_ECHO_REQ;
    mia_buf[MIA_ICMP_CODE] = 0;
    memset(mia_ptr(MIA_ICMP_CSUM), 0, 4);   /* also sets the identifier to 0 */
    mia_ston(MIA_ICMP_ECHO_SEQ, ping_seq++);
    memcpy(mia_ptr(MIA_ICMP_ECHO_DATA), &now, 4);
    mia_ston(MIA_ICMP_CSUM,
         ~inet_csum(0, mia_ptr(MIA_ICMP_POS), MIA_ICMP_HDR_LEN + 4));
    /* send out the packet */
    res = ip_send(addr,  PROTNUM_ICMP, MIA_ICMP_HDR_LEN + 4);

    mutex_unlock(&lock);
    return res;
}

void mia_udp_reply(uint16_t len)
{
    uint8_t tmp[2];

    /* switch UDP ports, set csum to zero and adjust length */
    memcpy(tmp, mia_ptr(MIA_UDP_SRC), len);
    memcpy(mia_ptr(MIA_UDP_SRC), mia_ptr(MIA_UDP_DST), len);
    memcpy(mia_ptr(MIA_UDP_DST), tmp, len);
    memset(mia_ptr(MIA_UDP_CSUM), 0, 2);
    mia_ston(MIA_UDP_LEN, len + MIA_UDP_HDR_LEN);

    ip_reply(len + MIA_UDP_HDR_LEN);
}

int mia_udp_send(uint8_t *ip, uint16_t src, uint16_t dst,
                 uint8_t *data, size_t len)
{
    int res;

    mutex_lock(&lock);

    mia_ston(MIA_UDP_SRC, src);
    mia_ston(MIA_UDP_DST, dst);
    memset(mia_ptr(MIA_UDP_CSUM), 0, 2);
    mia_ston(MIA_UDP_LEN, len + MIA_UDP_HDR_LEN);
    memcpy(mia_ptr(MIA_APP_POS), data, len);
    res = ip_send(ip, PROTNUM_UDP, len + MIA_UDP_HDR_LEN);

    mutex_unlock(&lock);
    return res;
}

void mia_udp_bind(mia_bind_t *bindings)
{
    mutex_lock(&lock);
    udp_bindings = bindings;

    int i = 0;
    while (udp_bindings && udp_bindings[i].port != 0) {
        printf("port: %i binds %p\n", udp_bindings[i].port, udp_bindings[i].cb);
        ++i;
    }
    mutex_unlock(&lock);
}
