/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application for demonstrating the RIOT network stack
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "thread.h"
#include "xtimer.h"
#include "net/emcute.h"
#include "net/gnrc/netif.h"
#include "net/ipv6/addr.h"
#include "pktcnt.h"

#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)

#define I3_ID               "i3-gas-sensor"
#define I3_TOPIC            "/i3/gasval"
#ifndef I3_BROKER
#define I3_BROKER           "affe::1"
#endif
#define I3_INTERVAL         (1 * US_PER_SEC)
#define I3_PORT             EMCUTE_DEFAULT_PORT


static char stack[THREAD_STACKSIZE_DEFAULT];
static const char *payload = "{\"id\":\"0x12a77af232\",\"val\":3000}";

static void *emcute_thread(void *arg)
{
    (void)arg;
    emcute_run(I3_PORT, I3_ID);
    return NULL;    /* should never be reached */
}

int main(void)
{
    /* init pktcnt */
    if (pktcnt_init() != PKTCNT_OK) {
        puts("error: unable to initialize pktcnt");
        return 1;
    }

    sock_udp_ep_t gw = { .family = AF_INET6, .port = I3_PORT };
    emcute_topic_t t = { I3_TOPIC, 0 };
    bool unbootstrapped = true;
    unsigned flags = 0;

#ifdef I3_CONFIRMABLE
    flags = EMCUTE_QOS_1;
#else
    flags = EMCUTE_QOS_0;
#endif

    puts("pktcnt: MQTT-SN push setup\n");

    /* wait for network to be set-up */
    while (unbootstrapped) {
        ipv6_addr_t addrs[GNRC_NETIF_IPV6_ADDRS_NUMOF];
        gnrc_netif_t *netif = gnrc_netif_iter(NULL);
        int res;

        xtimer_sleep(1);
        if ((res = gnrc_netif_ipv6_addrs_get(netif, addrs, sizeof(addrs))) > 0) {
            for (unsigned i = 0; i < (res / sizeof(ipv6_addr_t)); i++) {
                if (!ipv6_addr_is_link_local(&addrs[i])) {
                    char addr_str[IPV6_ADDR_MAX_STR_LEN];
                    printf("Global address %s configured\n",
                           ipv6_addr_to_str(addr_str, &addrs[i],
                                            sizeof(addr_str)));
                    unbootstrapped = false;
                    break;
                }
            }
        }
    }
    /* start the emcute thread */
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0,
                  emcute_thread, NULL, "emcute");

    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, I3_BROKER) == NULL) {
        printf("error parsing the broker's IPv6 address\n");
        return 1;
    }

    if (emcute_con(&gw, true, NULL, NULL, 0, flags) != EMCUTE_OK) {
        printf("error: unable to connect to [%s]:%i\n", I3_BROKER, (int)gw.port);
        return 1;
    }
    printf("successfully connected to broker at [%s]:%u\n", I3_BROKER, gw.port);

    /* now register out topic */
    if (emcute_reg(&t) != EMCUTE_OK) {
        printf("error: unable to register topic\n");
        return 1;
    }

    while (1) {
        xtimer_usleep(I3_INTERVAL);

        /* publish sensor data */
        if (emcute_pub(&t, payload, strlen(payload), flags) != EMCUTE_OK) {
            puts("error: failed to publish data");
        }
        else {
            puts("published sensor data");
        }

    }

    /* should be never reached */
    return 0;
}
