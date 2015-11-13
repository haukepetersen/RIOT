

#ifndef MIA_H
#define MIA_H

#include <stdint.h>
#include <string.h>

#include "net/netdev2.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIA_MSG_ISR         (0xabcd)
#define MIA_MSG_QUEUESIZE   (8U)

#define MIA_BUFSIZE         (1500U)

#define MIA_ARP_CACHE_SIZE  (8U)

#define MIA_DHCP            (0U)
#define MIA_DHCP_PORT       (67)

#define MIA_COAP            (0U)
#define MIA_COAP_PORT       (5683U)

#define MIA_MQTTSN          (0U)
#define MIA_MQTTSN_PORT     (1883U)

/**
 * @brief   Default header lengths
 * @{
 */
#define MIA_ETH_HDR_LEN     (14U)
#define MIA_ARP_HDR_LEN     (28U)
#define MIA_ICMP_HDR_LEN    (8U)
#define MIA_IP_HDR_LEN      (20U)
#define MIA_UDP_HDR_LEN     (8U)
/** @} */

/**
 * @brief   Base protocol field offsets
 * @{
 */
#define MIA_ETH_POS             (0U)
#define MIA_ETH_SRC             (MIA_ETH_POS + 6U)
#define MIA_ETH_DST             (MIA_ETH_POS + 0U)
#define MIA_ETH_TYPE            (MIA_ETH_POS + 12U)

#define MIA_ARP_POS             (14U)
#define MIA_ARP_HTYPE           (MIA_ARP_POS + 0U)
#define MIA_ARP_PTYPE           (MIA_ARP_POS + 2U)
#define MIA_ARP_HLEN            (MIA_ARP_POS + 4U)
#define MIA_ARP_PLEN            (MIA_ARP_POS + 5U)
#define MIA_ARP_OPER            (MIA_ARP_POS + 6U)
#define MIA_ARP_SHA             (MIA_ARP_POS + 8U)
#define MIA_ARP_SPA             (MIA_ARP_POS + 14U)
#define MIA_ARP_THA             (MIA_ARP_POS + 18U)
#define MIA_ARP_TPA             (MIA_ARP_POS + 24U)

#define MIA_IP_POS              (14U)
#define MIA_IP_VER              (MIA_IP_POS + 0U)
#define MIA_IP_LEN              (MIA_IP_POS + 2U)
#define MIA_IP_TTL              (MIA_IP_POS + 8U)
#define MIA_IP_PROTO            (MIA_IP_POS + 9U)
#define MIA_IP_CSUM             (MIA_IP_POS + 10U)
#define MIA_IP_SRC              (MIA_IP_POS + 12U)
#define MIA_IP_DST              (MIA_IP_POS + 16U)

#define MIA_ICMP_POS            (34U)
#define MIA_ICMP_TYPE           (MIA_ICMP_POS + 0U)
#define MIA_ICMP_CODE           (MIA_ICMP_POS + 1U)
#define MIA_ICMP_CSUM           (MIA_ICMP_POS + 2U)
#define MIA_ICMP_ECHO_ID        (MIA_ICMP_POS + 4U)
#define MIA_ICMP_ECHO_SEQ       (MIA_ICMP_POS + 6U)
#define MIA_ICMP_ECHO_DATA      (MIA_ICMP_POS + 8U)

#define MIA_UDP_POS             (34U)
#define MIA_UDP_SRC             (MIA_UDP_POS + 0U)
#define MIA_UDP_DST             (MIA_UDP_POS + 2U)
#define MIA_UDP_LEN             (MIA_UDP_POS + 4U)
#define MIA_UDP_CSUM            (MIA_UDP_POS + 6U)

#define MIA_APP_POS             (42U)
/** @} */

/**
 * @brief   Custom MIA error codes
 * @{
 */
#define MIA_ERR_NO_ROUTE        (-50)
#define MIA_ERR_OVERFLOW        (-51)
/** @} */

extern uint8_t mia_buf[];

typedef void (*mia_cb_t)(void);

typedef struct {
    uint16_t port;
    mia_cb_t cb;
} mia_bind_t;


int mia_run(netdev2_t *dev);

void mia_ip_config(uint8_t *addr, uint8_t mask, uint8_t *gw);

int mia_ping(uint8_t *addr, mia_cb_t cb);

void mia_udp_bind(mia_bind_t *bindings);
int mia_udp_send(uint8_t *ip, uint16_t src, uint16_t dst,
                 uint8_t *data, size_t len);

void mia_dump_ip(const int pos);
void mia_dump_mac(const int pos);


static inline uint8_t *mia_ptr(const int pos)
{
    return mia_buf + pos;
}

static inline uint16_t mia_ntos(const int pos)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (uint16_t)((mia_buf[pos] << 8) | mia_buf[pos + 1]);
#else
    return (uint16_t)((mia_buf[pos + 1] << 8) | mia_buf[pos]);
#endif
}

static inline void mia_ston(const int pos, uint16_t val)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    mia_buf[pos] = (uint8_t)(val >> 8);
    mia_buf[pos + 1] = (uint8_t)val;
#else
    mia_buf[pos] = (uint8_t)val;
    mia_buf[pos + 1] = (uint8_t)(val >> 8);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* MIA_H */
