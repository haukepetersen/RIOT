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
 * @brief       Example application running the MIA network stack
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "thread.h"
#include "shell.h"
#include "xtimer.h"

#include "net/mia.h"
#include "net/mia/ip.h"
#include "net/mia/udp.h"
#include "net/mia/icmp.h"
#include "net/mia/dhcp.h"
#include "net/mia/coap.h"
#include "net/mia/print.h"


#define STACKSIZE           (THREAD_STACKSIZE_DEFAULT)
#define MIA_PRIO            (THREAD_PRIORITY_MAIN - 4)

#define PING_DELAY          (1000 * 1000U)
#define DHCP_DELAY          (200 * 1000U)

#define NOT_NATIVE 0

#if NOT_NATIVE
#include "enc28j60.h"
#else
#include "netdev2_tap.h"
#endif

#if NOT_NATIVE
static enc28j60_params_t encp = {
    SPI_1,
    GPIO_PIN(PORT_B, 12),
    GPIO_PIN(PORT_B, 11),
    GPIO_PIN(PORT_B, 10),
};

static enc28j60_t dev;
#else
extern netdev2_tap_t netdev2_tap;
#endif

static char stack[STACKSIZE];


static void *run_mia_run(void *arg)
{
    (void)arg;
    puts("MIA: starting the stack now");
#if NOT_NATIVE
    mia_run((netdev2_t *)&dev);
#else
    mia_run((netdev2_t *)&netdev2_tap);
#endif
    return NULL;
}

static void dump_time(uint32_t dt)
{
    int ms = (dt / 1000);
    int us = (dt - (ms * 1000));
    printf("%i.%03ims", ms, us);
}

static void ping_cb(void)
{
    uint32_t now = xtimer_now();
    uint32_t then;
    memcpy(&then, mia_ptr(MIA_ICMP_ECHO_DATA), 4);

    printf("%i bytes from ", mia_ntos(MIA_IP_LEN) - MIA_IP_HDR_LEN);
    mia_print_ip_addr(mia_ptr(MIA_IP_SRC));
    printf(": imcp_seq=%i ttl=%i time=",
           mia_ntos(MIA_ICMP_ECHO_SEQ), mia_buf[MIA_IP_TTL]);
    dump_time(now - then);
    puts("");
}

static void get_ip(char **argv, uint8_t *ip)
{
    ip[0] = (uint8_t)atoi(argv[1]);
    ip[1] = (uint8_t)atoi(argv[2]);
    ip[2] = (uint8_t)atoi(argv[3]);
    ip[3] = (uint8_t)atoi(argv[4]);
}

static int cmd_ifconfig(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("eth0\tLink encap:Ethernet  HWaddr ");
    mia_print_mac_addr(mia_mac);
    printf("\n\tinet addr:");
    mia_print_ip_addr(mia_ip_addr);
    printf("  Bcast:");
    mia_print_ip_addr(mia_ip_bcast);
    printf("  Mask:/%i  Gateway:", (8 * mia_ip_mask));
    mia_print_ip_addr(mia_ip_gateway);
    puts("");
    return 0;
}

static int cmd_conf(int argc, char **argv)
{
    if (argc < 10) {
        printf("usage: %s addr0 addr1 addr2 addr3 mask gw0 gw1 gw2 gw3\n", argv[0]);
        return 1;
    }
    for (int i = 0; i < 4; i++) {
        mia_ip_addr[i] = (uint8_t)atoi(argv[i + 1]);
    }
    mia_ip_mask = (uint8_t)atoi(argv[5]);
    mia_ip_mask = (mia_ip_mask > 3) ? 3 : mia_ip_mask;
    mia_ip_mask = (mia_ip_mask < 1) ? 1 : mia_ip_mask;
    for (int i = 0; i < mia_ip_mask; i++) {
        mia_ip_bcast[i] = mia_ip_addr[i];
    }
    for (int i = mia_ip_mask; i < 4; i++) {
        mia_ip_bcast[i] = 255;
    }
    for (int i = 0; i < 4; i++) {
        mia_ip_gateway[i] = (uint8_t)atoi(argv[i + 6]);
    }
    return 0;
}

static int cmd_ping(int argc, char **argv)
{
    uint8_t ip[4];
    int count = 3;

    if (argc < 5) {
        printf("usage: %s addr0 addr1 addr2 addr3 [count]\n", argv[0]);
        return 1;
    }
    get_ip(argv, ip);

    if (argc > 5) {
        count = atoi(argv[5]);
    }

    for (int i = 0; i < count; i++) {
        mia_icmp_ping(ip, ping_cb);
        xtimer_usleep(PING_DELAY);
    }
    return 0;
}

static int cmd_udp(int argc, char **argv)
{
    uint8_t ip[4];
    uint16_t src, dst;
    size_t len;

    if (argc < 8) {
        printf("usage: %s addr0 addr1 addr2 addr3 src-port dst-port data\n",
               argv[0]);
        return 1;
    }
    get_ip(argv, ip);
    src = (uint16_t)atoi(argv[5]);
    dst = (uint16_t)atoi(argv[6]);

    len = strlen(argv[7]);
    memcpy(mia_ptr(MIA_APP_POS), argv[7], len);
    mia_buf[MIA_APP_POS + len] = '\n';
    mia_ston(MIA_UDP_LEN, len + 1 + MIA_UDP_HDR_LEN);
    mia_udp_send(ip, src, dst);
    return 0;
}

static int cmd_dhcp(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    mia_dhcp_request();
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "ifconfig", "show device infos", cmd_ifconfig },
    { "conf", "configure IP addresses", cmd_conf },
    { "ping", "ping some other host", cmd_ping },
    { "udp", "send data via UDP", cmd_udp },
    { "dhcp", "dhcp request", cmd_dhcp },
    { NULL, NULL, NULL }
};

static void udp_info(void)
{
    printf("UDP: from ");
    mia_print_ip_addr(mia_ptr(MIA_IP_SRC));
    printf(", src-port:%i, dst-port:%i\n",
           mia_ntos(MIA_UDP_SRC), mia_ntos(MIA_UDP_DST));
}

static void udp_print(void)
{
    udp_info();
    for (uint16_t i = 0; i < (mia_ntos(MIA_UDP_LEN) - MIA_UDP_HDR_LEN); i++) {
        printf("%c", *mia_ptr(MIA_APP_POS + i));
    }
    puts("\n~~~EOF~~~");
}

static void udp_echo(void)
{
    udp_info();
    mia_udp_reply();
}

const mia_bind_t mia_udp_bindings[] = {
    { 443, udp_print },
    { 80, udp_echo },
    { DHCP_CLI_PORT, mia_dhcp_process },
    { COAP_PORT, mia_coap_process },
    { 0, NULL }
};

int main(void)
{
    puts("Hello MIA!");

#if NOT_NATIVE
    /* initialize out network device */
    enc28j60_setup(&dev, &encp);
#endif

    /* run MIA */
    thread_create(stack, sizeof(stack), MIA_PRIO, THREAD_CREATE_STACKTEST,
                  run_mia_run, NULL, "mia");

    /* trying to get an IP via DHCP */
#if NOT_NATIVE
    while (mia_ip_mask == 0) {
        puts("Requesting IP via DHCP...");
        mia_dhcp_request();
        xtimer_usleep(DHCP_DELAY);
    }
    puts("Successfully IP configuration via DHCP:");
#else
    puts("not doing DHCP");
#endif
    cmd_ifconfig(0, NULL);

    puts("Starting the shell now...");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
