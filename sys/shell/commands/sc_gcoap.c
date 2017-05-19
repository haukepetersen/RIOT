/*
 * Copyright (C) 2015-2016 Ken Bannister. All rights reserved.
 *               2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_shell_commands
 * @{
 *
 * @file
 * @brief       gcoap CLI shell command
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "net/gcoap.h"

static const char *req_methods[] = { "get", "post", "put" };

// static uint8_t buf[GCOAP_PDU_BUF_SIZE];

static unsigned req_count = 0;

// static void resp_handler(unsigned req_state, coap_pkt_t* pdu)
// {
//     if (req_state == GCOAP_MEMO_TIMEOUT) {
//         printf("gcoap: timeout for msg ID %02u\n", coap_get_id(pdu));
//         return;
//     }
//     else if (req_state == GCOAP_MEMO_ERR) {
//         printf("gcoap: error in response\n");
//         return;
//     }

//     char *class_str = (coap_get_code_class(pdu) == COAP_CLASS_SUCCESS)
//                             ? "Success" : "Error";
//     printf("gcoap: response %s, code %1u.%02u", class_str,
//                                                 coap_get_code_class(pdu),
//                                                 coap_get_code_detail(pdu));
//     printf(", payload legth: %u\n", (unsigned)pdu->payload_len);
// }

static void usageinfo(const char *cmd, const char *sub)
{
    if (sub) {
        printf("usage: %s %s <addr> <port> <path> [payload]\n", cmd, sub);
    }
    else {
        printf("usage: %s {get|post|put|info}\n", cmd);
    }
}

static void coapinfo(void)
{
    uint8_t open_reqs;
    gcoap_op_state(&open_reqs);

    printf("CoAP server is listening on port %u\n", (unsigned)GCOAP_PORT);
    printf(" CLI requests sent: %u\n", req_count);
    printf("CoAP open requests: %u\n", (unsigned)open_reqs);
}

static void make_request(unsigned type, const char *addr_str, const char *port_str,
                         char *path, const char *payload)
{
    size_t bytes_sent;
    coap_pkt_t pdu;
    size_t len;
    sock_udp_ep_t remote = SOCK_IPV6_EP_ANY;

    /* parse destination address */
    if (ipv6_addr_from_str((ipv6_addr_t *)remote.addr.ipv6, addr_str) == NULL) {
        puts("error: unable to parse destination address");
        return 0;
    }

    /* parse port */
    remote.port = atoi(port_str);
    if (remote.port == 0) {
        puts("error: unable to parse destination port");
        return 0;
    }

    /* prepare CoAP packet */
    if (payload) {
        gcoap_req_init(&pdu, buf, (size_t)GCOAP_PDU_BUF_SIZE, (type + 1), path);
        memcpy(pdu.payload, payload, strlen(payload));
        len = gcoap_finish(&pdu, strlen(payload), COAP_FORMAT_TEXT);
    }
    else {
        len = gcoap_request(&pdu, buf, GCOAP_PDU_BUF_SIZE, (type + 1), path);
    }

    /* and send it */
    bytes_sent = gcoap_req_send2(buf, len, &remote, resp_handler);
    if (bytes_sent >= 0) {
        ++req_count;
        printf("successfully send out %i byte\n", (int)bytes_sent);
    }

}

int _gcoap_coap(int argc, char **argv)
{
    int hit = 0;

    if (argc > 1) {
        if (strcmp(argv[1], "info") == 0) {
            coapinfo();
            hit = 1;
        }
        for (unsigned i = 0;
             i < (sizeof(req_methods) / sizeof(const char *));
             i++) {
            if (strcmp(argv[1], req_methods[i]) == 0) {
                if (argc < 5) {
                    usageinfo(argv[0], argv[1]);
                }
                else {
                    const char *payload = (argc >= 6) ? argv[5] : NULL;
                    make_request(i, argv[2], argv[3], argv[4], payload);
                }
                hit = 1;
            }
        }
    }

    if (!hit) {
        usageinfo(argv[0], NULL);
    }

    return 0;
}
