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
#include "enc28j60.h"
#include "net/mia.h"

#define STACKSIZE           (THREAD_STACKSIZE_DEFAULT)
#define MIA_PRIO            (THREAD_PRIORITY_MAIN - 4)

#define PING_DELAY          (1000 * 1000U)

#define IP_ADDR             {192, 168, 23, 199}
#define IP_MASK             3       /* 3 * 8 --> 24 */
#define IP_GATEWAY          {192, 168, 23, 1}

static enc28j60_params_t encp = {
    SPI_1,
    GPIO_PIN(PORT_B, 12),
    GPIO_PIN(PORT_B, 11),
    GPIO_PIN(PORT_B, 10),
};

static char stack[STACKSIZE];

static enc28j60_t dev;

static uint8_t ip[] = IP_ADDR;
static uint8_t gw[] = IP_GATEWAY;

static void *run_mia_run(void *arg)
{
    (void)arg;
    puts("MIA: starting the stack now");
    mia_run((netdev2_t *)&dev);
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
    mia_dump_ip(MIA_IP_SRC);
    printf(": imcp_seq=%i ttl=%i time=",
           mia_ntos(MIA_ICMP_ECHO_SEQ), mia_ntos(MIA_IP_TTL));
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
        mia_ping(ip, ping_cb);
        xtimer_usleep(PING_DELAY);
    }
    return 0;
}

static int cmd_udp(int argc, char **argv)
{
    uint8_t ip[4];
    uint16_t src, dst;

    if (argc < 8) {
        printf("usage: %s addr0 addr1 addr2 addr3 src-port dst-port data\n",
               argv[0]);
        return 1;
    }
    get_ip(argv, ip);
    src = (uint16_t)atoi(argv[5]);
    dst = (uint16_t)atoi(argv[6]);

    mia_udp_send(ip, src, dst, (uint8_t *)argv[7], strlen(argv[7]));
    return 0;
}

static void bind_echo(void)
{
    printf("UDP: from ");
    mia_dump_ip(MIA_IP_SRC);
    printf(", src-port:%i, dst-port:%i\n~~~\n",
           mia_ntos(MIA_UDP_SRC), mia_ntos(MIA_UDP_DST));
    for (uint16_t i = 0; i < (mia_ntos(MIA_UDP_LEN) - MIA_UDP_HDR_LEN); i++) {
        printf("%c", *mia_ptr(MIA_APP_POS + i));
    }
    printf("\n~~~\n");
}

static const shell_command_t shell_commands[] = {
    { "ping", "ping some other host", cmd_ping },
    { "udp", "send data via UDP", cmd_udp },
    { NULL, NULL, NULL }
};

static const mia_bind_t udp_bindings[] = {
    { 443, bind_echo },
    { 80, bind_echo },
    { 0, NULL }
};

int main(void)
{
    puts("Hello MIA!");

    /* initialize out network device */
    enc28j60_setup(&dev, &encp);
    /* run MIA */
    thread_create(stack, sizeof(stack), MIA_PRIO, CREATE_STACKTEST,
                  run_mia_run, NULL, "mia");

    puts("MIA: configuring the stack");
    mia_ip_config(ip, 3, gw);
    mia_udp_bind((mia_bind_t *)udp_bindings);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
