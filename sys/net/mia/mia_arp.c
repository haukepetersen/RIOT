

#include "net/ethernet.h"
#include "net/ethertype.h"

#include "net/mia.h"
#include "net/mia/ip.h"
#include "net/mia/eth.h"
#include "net/mia/arp.h"
#include "net/mia/arp_cache.h"

#define ENABLE_DEBUG            (0)
#include "debug.h"

#define ARP_OPER_REQ            (1U)
#define ARP_OPER_RPLY           (2U)


static const uint8_t _arp_head[] = {0x00, 0x01, 0x08, 0x00, 0x06, 0x04};


void mia_arp_process(void)
{
    /* check hardware type and protocol type as well as HLEN and PLEN*/
    if (memcmp(mia_ptr(MIA_ARP_POS), _arp_head, 6) != 0) {
        DEBUG("[mia] arp: error: packet is not IPv4 over Ethernet\n");
        return;
    }

    /* insert new information into ARP cache */
    mia_arp_cache_insert(mia_ptr(MIA_ARP_SPA), mia_ptr(MIA_ARP_SHA));
    DEBUG("[mia] arp: inserted new address to ARP cache\n");

    if (mia_ntos(MIA_ARP_OPER) == ARP_OPER_REQ) {
        /* was the request for my IP address? */
        if (memcmp(mia_ptr(MIA_ARP_TPA), mia_ip_addr, MIA_IP_ADDR_LEN) == 0) {
            DEBUG("[mia] arp: got request for my IP address\n");
            mia_ston(MIA_ARP_OPER, ARP_OPER_RPLY);
            memcpy(mia_ptr(MIA_ARP_THA), mia_ptr(MIA_ARP_SHA),
                   ETHERNET_ADDR_LEN + MIA_IP_ADDR_LEN);
            memcpy(mia_ptr(MIA_ARP_SHA), mia_mac, ETHERNET_ADDR_LEN);
            memcpy(mia_ptr(MIA_ARP_SPA), mia_ip_addr, MIA_IP_ADDR_LEN);
            mia_eth_reply(MIA_ARP_HDR_LEN);
        }
    }
}

void mia_arp_request(const uint8_t *ip)
{
    /* build ARP header */
    memcpy(mia_ptr(MIA_ARP_POS), _arp_head, 6);
    mia_ston(MIA_ARP_OPER, ARP_OPER_REQ);
    memcpy(mia_ptr(MIA_ARP_SHA), mia_mac, ETHERNET_ADDR_LEN);
    memcpy(mia_ptr(MIA_ARP_SPA), mia_ip_addr, MIA_IP_ADDR_LEN);
    memset(mia_ptr(MIA_ARP_THA), 0, ETHERNET_ADDR_LEN);
    memcpy(mia_ptr(MIA_ARP_TPA), ip, MIA_IP_ADDR_LEN);
    /* send packet */
    mia_eth_send((uint8_t *)mia_bcast, ETHERTYPE_ARP, MIA_ARP_HDR_LEN);
    DEBUG("[mia] arp: request was send\n");
}
