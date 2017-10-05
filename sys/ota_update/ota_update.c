/*
 * Copyright (C) 2017 Kaspar Schleiser <kaspar@schleiser.de>
 *               2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys
 * @{
 *
 * @file
 * @brief       Over the air (OTA) upgrade module
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Francisco Acosta <francisco.acosta@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "thread.h"
#include "net/gcoap.h"
#include "ota_update.h"
#include "firmware_update.h"
#include "xtimer.h"
#include "sema.h"
#include "log.h"

static ota_request_t ota_request;

static kernel_pid_t ota_update_pid = KERNEL_PID_UNDEF;
static char _msg_stack[OTA_UPDATE_STACK];
static msg_t _ota_update_msg_queue[OTA_UPDATE_MSG_QUEUE_SIZE];

static uint16_t remote_version = 0;
static uint16_t local_version;
static uint32_t local_appid;
static char update_name[FIRMWARE_NAME_LENGTH];

/* Counts requests sent by ota_update. */
static uint16_t req_count = 0;
static sema_t sema_fw_version = SEMA_CREATE_LOCKED();
static sema_t sema_fw_name = SEMA_CREATE_LOCKED();

#ifdef MODULE_OTA_UPDATE_TFTP
extern void ota_update_tftp_client_start(uint8_t firmware_slot, char *name, char *server_ip_address);
#endif /* MODULE_OTA_UPDATE_TFTP */

/*
 * Response callback.
 */
static void _resp_handler(unsigned req_state, coap_pkt_t* pdu,
                          sock_udp_ep_t *remote)
{
    (void)remote;       /* not interested in the source currently */

    if (req_state == GCOAP_MEMO_TIMEOUT) {
        printf("[ota_update] timeout for msg ID %02u\n", coap_get_id(pdu));
        return;
    }
    else if (req_state == GCOAP_MEMO_ERR) {
        printf("[ota_update] error in response\n");
        return;
    }

    char *class_str = (coap_get_code_class(pdu) == COAP_CLASS_SUCCESS)
                            ? "Success" : "Error";
    printf("[ota_update] %s, response code %1u.%02u", class_str,
                                                coap_get_code_class(pdu),
                                                coap_get_code_detail(pdu));
    if (pdu->payload_len) {
        char remote_version_str[10] = {0};
        printf(", %u bytes :%.*s\n", pdu->payload_len, pdu->payload_len,
                                                      (char *)pdu->payload);
        switch (ota_request) {
            case COAP_GET_VERSION:
                strncpy(remote_version_str, (char *)pdu->payload, pdu->payload_len);
                remote_version = strtol(remote_version_str, NULL, 16);
                printf("[ota_update] Got remote version %#x\n", remote_version);
                sema_post(&sema_fw_version);
                break;
            case COAP_GET_NAME:
                strncpy(update_name, (char *)pdu->payload, pdu->payload_len);
                printf("[ota_update] Got remote name %s\n", update_name);
                sema_post(&sema_fw_name);
                break;
        }
    }
    else {
        printf(", empty payload\n");
    }
}

static size_t _send(uint8_t *buf, size_t len, char *addr_str, char *port_str)
{
    ipv6_addr_t addr;
    size_t bytes_sent;
    sock_udp_ep_t remote;

    remote.family = AF_INET6;
    remote.netif  = SOCK_ADDR_ANY_NETIF;

    /* parse destination address */
    if (ipv6_addr_from_str(&addr, addr_str) == NULL) {
        puts("[ota_update] unable to parse destination address");
        return 0;
    }
    memcpy(&remote.addr.ipv6[0], &addr.u8[0], sizeof(addr.u8));

    /* parse port */
    remote.port = atoi(port_str);
    if (remote.port == 0) {
        puts("[ota_update] unable to parse destination port");
        return 0;
    }

    bytes_sent = gcoap_req_send2(buf, len, &remote, _resp_handler);
    if (bytes_sent > 0) {
        req_count++;
    }
    return bytes_sent;
}

int ota_update_stop(void)
{
    /* check if there is a server running */
    if (ota_update_pid == KERNEL_PID_UNDEF) {
        LOG_DEBUG("[ota_update] no OTA client running\n");
        return -1;
    }

    /* prepare the stop message */
    msg_t m = {
        thread_getpid(),
        OTA_UPDATE_STOP,
        { NULL }
    };

    /* send the stop message */
    msg_send(&m, ota_update_pid);

    return 0;
}

static void *ota_update_start(void *arg)
{
    (void)arg;
    msg_t msg;
    msg_init_queue(_ota_update_msg_queue, OTA_UPDATE_MSG_QUEUE_SIZE);

    /* Wait a bit to initialise network addresses */
    xtimer_sleep(5);

    coap_pkt_t pdu;
    firmware_metadata_t metadata;
    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    char coap_resource[32];
    int current_slot = firmware_current_slot();
    int target_slot = firmware_target_slot();

    memcpy(&metadata, firmware_get_metadata(current_slot), sizeof(firmware_metadata_t));

    if (firmware_validate_metadata_checksum(&metadata) != -1) {
        local_appid = metadata.appid;
        local_version = metadata.version;
        firmware_metadata_print(&metadata);
        printf("[ota_update] Server address: %s\n", OTA_SERVER_ADDRESS);
    }
    else {
        puts("[ota_update] Error! Cannot retrieve local metadata\n");
        return 0;
    }

    while(1) {
        xtimer_msg_receive_timeout(&msg, OTA_PERIODIC_REQ_TIME);

        /* check if the ota_update stop message has been received */
        if (msg.type == OTA_UPDATE_STOP) {
            break;
        }

        size_t len;
        ota_request = 0;
        memset(coap_resource, 0, sizeof(coap_resource));
        sprintf(coap_resource, "/0x%lX/version", local_appid);

        printf("[ota_update] Requesting resource %s\n", coap_resource);
        len = gcoap_request(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_METHOD_GET,
                            coap_resource);
        printf("[ota_update] sending msg ID %u, %u bytes\n", coap_get_id(&pdu),
               (unsigned) len);
        if (!_send(&buf[0], len, OTA_SERVER_ADDRESS, OTA_SERVER_COAP_PORT)) {
            puts("[ota_update] msg send failed");
        }

        if (sema_wait_timed(&sema_fw_version, OTA_REQ_TIMEOUT)
            == -ETIMEDOUT) {
            printf("[ota_update] Request %u timed out\n", coap_get_id(&pdu));
        }
        else {

            if (local_version >= remote_version) {
                puts("[ota_update] No new firmware available");
            }
            else {
                ota_request = 1;
                memset(coap_resource, 0, sizeof(coap_resource));
                sprintf(coap_resource, "/0x%lX/slot%d/name", local_appid, target_slot);
                printf("[ota_update] Requesting resource %s\n", coap_resource);
                len = gcoap_request(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_METHOD_GET,
                                    coap_resource);
                printf("[ota_update] sending msg ID %u, %u bytes\n", coap_get_id(&pdu),
                       (unsigned) len);
                if (!_send(&buf[0], len, OTA_SERVER_ADDRESS, OTA_SERVER_COAP_PORT)) {
                    puts("[ota_update] msg send failed");
                }

                if (sema_wait_timed(&sema_fw_name, OTA_REQ_TIMEOUT)
                    == -ETIMEDOUT) {
                    printf("[ota_update] Request %u timed out\n", coap_get_id(&pdu));
                }
                else {
#ifdef MODULE_OTA_UPDATE_TFTP
                    ota_update_tftp_client_start(target_slot, update_name, OTA_SERVER_ADDRESS);
#endif
                }
            }
        }
    }

    /* the OTA client has been stopped */
    puts("[ota_update] Client stopped");

    return 0;
}

kernel_pid_t ota_update_init(void)
{
    if (ota_update_pid != KERNEL_PID_UNDEF) {
        return -EEXIST;
    }

    ota_update_pid = thread_create(_msg_stack, sizeof(_msg_stack), THREAD_PRIORITY_MAIN - 1,
                                   THREAD_CREATE_STACKTEST, ota_update_start, NULL, "ota_update");

    return ota_update_pid;
}
