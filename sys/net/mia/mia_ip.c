
#include "net/ethertype.h"
#include "net/protnum.h"
#include "net/inet_csum.h"

#include "net/mia.h"
#include "net/mia/eth.h"
#include "net/mia/arp.h"
#include "net/mia/arp_cache.h"
#include "net/mia/udp.h"
#include "net/mia/icmp.h"

#define ENABLE_DEBUG            (0)
#include "debug.h"


#define IP_ADDR_LEN             (4U)
#define IP_VER_VAL              (0x45)
#define IP_DEFAULT_TTL          (64U)



uint32_t mia_ip_addr = 0;
uint32_t mia_ip_mask = 0;
uint32_t mia_ip_bcast = 0xffffffff;
uint32_t mia_ip_gateway = 0xffffffff;


static void ip_csum(uint16_t len)
{
    mia_ston(MIA_IP_LEN, len);
    memset(mia_ptr(MIA_IP_CSUM), 0, 2);
    mia_ston(MIA_IP_CSUM, ~inet_csum(0, mia_ptr(MIA_IP_POS), MIA_IP_HDR_LEN));
}


void mia_ip_process(void)
{
    /* packet for me? */
    if (!(memcmp(mia_ptr(MIA_IP_DST), mia_ip_addr, MIA_IP_ADDR_LEN) == 0 ||
          memcmp(mia_ptr(MIA_IP_DST), mia_bcast, MIA_IP_ADDR_LEN) == 0)) {
        DEBUG("[mia] ip:  got packet that was not for me\n");
        return;
    }

    switch (mia_buf[MIA_IP_PROTO]) {
        case PROTNUM_ICMP:
            mia_icmp_process();
            break;
        case PROTNUM_UDP:
            mia_udp_process();
            break;
    }
}

void mia_ip_reply(uint16_t len)
{
    memcpy(mia_ptr(MIA_IP_DST), mia_ptr(MIA_IP_SRC), MIA_IP_ADDR_LEN);
    memcpy(mia_ptr(MIA_IP_SRC), mia_ip_addr, MIA_IP_ADDR_LEN);
    ip_csum(MIA_IP_HDR_LEN + len);
    mia_eth_reply(MIA_IP_HDR_LEN + len);
}

int mia_ip_send(uint8_t *ip, uint8_t proto, uint16_t len)
{
    uint8_t *ta, *mac;

    if (memcmp(ip, mia_bcast, MIA_IP_ADDR_LEN) == 0) {
        mac = (uint8_t *)mia_bcast;
    }
    else {
        ta = (memcmp(ip, mia_ip_addr, mia_ip_mask) == 0) ? ip : mia_ip_gateway;
        mac = mia_arp_cache_lookup(ta);
        if (mac == NULL) {
            mia_arp_request(ta);
            DEBUG("[mia] ip: host not in ARP cache, requesting now\n");
            return MIA_ERR_NO_ROUTE;
        }
    }

    /* fill the IPv4 header */
    mia_buf[MIA_IP_VER] = IP_VER_VAL;
    memset(mia_ptr(MIA_IP_VER + 1), 0, 7);
    mia_buf[MIA_IP_TTL] = IP_DEFAULT_TTL;
    mia_buf[MIA_IP_PROTO] = proto;
    memcpy(mia_ptr(MIA_IP_DST), ip, MIA_IP_ADDR_LEN);
    memcpy(mia_ptr(MIA_IP_SRC), mia_ip_addr, MIA_IP_ADDR_LEN);
    /* and finish by calculating the checksum */
    ip_csum(MIA_IP_HDR_LEN + len);
    /* finally send out the packet */
    mia_eth_send(mac, ETHERTYPE_IPV4, MIA_IP_HDR_LEN + len);
    return 0;
}
