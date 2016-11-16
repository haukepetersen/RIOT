
#include <stdio.h>

#include "net/protnum.h"
#include "net/ethertype.h"

#include "net/mia.h"
#include "net/mia/eth.h"
#include "net/mia/ip.h"
#include "net/mia/udp.h"

void mia_print_ip_addr(uint8_t *ip)
{
    printf("%i.%i.%i.%i", ip[0], ip[1], ip[2], ip[3]);
}

void mia_print_mac_addr(uint8_t *mac)
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void mia_print_pkt(const char *pre)
{
    int len = 0;
    if (mia_ntos(MIA_ETH_TYPE) == ETHERTYPE_IPV4) {
        len = (int)(mia_ntos(MIA_IP_LEN) + MIA_ETH_HDR_LEN);
    }
    else if (mia_ntos(MIA_ETH_TYPE) == ETHERTYPE_ARP) {
        len = MIA_ETH_HDR_LEN + MIA_ARP_HDR_LEN;
    }

    printf("%sdst: ", pre);
    mia_print_mac_addr(mia_ptr(MIA_ETH_DST));
    printf(" src: ");
    mia_print_mac_addr(mia_ptr(MIA_ETH_SRC));
    printf(" len: %i\n", len);
    printf("%stype:", pre);
    if (mia_ntos(MIA_ETH_TYPE) == ETHERTYPE_ARP) {
        printf("ARP\n");
    }
    else if (mia_ntos(MIA_ETH_TYPE) == ETHERTYPE_IPV4) {
        printf("IPv4\n");
    }
    else {
        printf("unknown\n");
        return;
    }
    if (mia_ntos(MIA_ETH_TYPE) == ETHERTYPE_IPV4) {
        printf("%ssrc:", pre);
        mia_print_ip_addr(mia_ptr(MIA_IP_SRC));
        printf(" dst:");
        mia_print_ip_addr(mia_ptr(MIA_IP_DST));
        printf("\n");
        printf("%stype:", pre);
        if (mia_buf[MIA_IP_PROTO] == PROTNUM_UDP) {
            printf("UDP\n");
            printf("%ssrc-port: %i, dst-port: %i\n", pre,
                   (int)mia_ntos(MIA_UDP_SRC), (int)mia_ntos(MIA_UDP_DST));
        }
        else if (mia_buf[MIA_IP_PROTO] == PROTNUM_ICMP) {
            printf("ICMP\n");
        }
        else {
            printf("unknown\n");
            return;
        }
    }
}
