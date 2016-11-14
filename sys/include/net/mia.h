/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_mia Minimalistic IP Application Stack
 * @ingroup     mia
 * @brief       MIA - a minimal IPv4 network stack
 *
 * This is MIA. It can do Ethernet, IPv4, and UDP only.
 *
 * Restrictions:
 * - only a single instance (network interface)
 *
 * @{
 * @file
 * @brief       MIA's main interface definition
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef MIA_H
#define MIA_H

#include <stdint.h>
#include <string.h>

#include "mutex.h"
#include "net/netdev2.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Basic stack configuration parameters
 */
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
#define MIA_ARP_HDR_LEN     (28U)
#define MIA_ICMP_HDR_LEN    (8U)
#define MIA_IP_HDR_LEN      (20U)
#define MIA_UDP_HDR_LEN     (8U)
/** @} */

/**
 * @brief   Base protocol field offsets
 * @{
 */
#define MIA_APP_POS             (42U)
/** @} */


#define MIA_IP_ADDR_LEN         (4U)

/**
 * @brief   Custom MIA error codes
 * @{
 */
#define MIA_ERR_NO_ROUTE        (-50)
#define MIA_ERR_OVERFLOW        (-51)
/** @} */

enum {
    MIA_OK      = 0,
    MIA_NOPORT  = -1
};

extern uint8_t mia_buf[];
extern mutex_t mia_mutex;

extern netdev2_t *mia_dev;

extern uint8_t mia_mac[];


extern const uint8_t mia_bcast[];


typedef void (*mia_cb_t)(void);

typedef struct mia_bind_t {
    struct mia_bind_t *next;
    mia_cb_t cb;
    uint16_t port;
} mia_bind_t;


int mia_run(netdev2_t *dev);

int mia_ping(uint8_t *addr, mia_cb_t cb);


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

static inline void mia_lock(void)
{
    mutex_lock(&mia_mutex);
}

static inline void mia_unlock(void)
{
    mutex_unlock(&mia_mutex);
}

#ifdef __cplusplus
}
#endif

#endif /* MIA_H */
