/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_stdio_udp
 * @{
 *
 * @file
 * @brief       STDIO via UDP implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "net/sock/udp.h"
#include "stdio_udp.h"

static sock_udp_t sock;
static sock_udp_ep_t remote;


void stdio_init(void)
{
    /* set the remote ep to some defined value */
    remote.port = 0;
}

void stdio_udp_init(void)
{
    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = STDIO_UDP_PORT;

    sock_udp_create(&sock, &local, NULL, 0);
}

int stdio_read(char* buffer, int count)
{
    ssize_t n;

    while ((n = sock_udp_recv(&sock, buffer, (size_t)count,
                              SOCK_NO_TIMEOUT, &remote)) <= 0) {}
    return (int)n;

}

int stdio_write(const char* buffer, int len)
{
    /* send data only if a remote host is known */
    if (remote.port != 0) {
        sock_udp_send(&sock, buffer, (size_t)len, &remote);
    }
    return len;
}
