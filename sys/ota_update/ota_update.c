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
#include "net/gnrc/tftp.h"
#include "firmware_update.h"
#include "net/gcoap.h"
#include "ota_update.h"
#include "xtimer.h"
#include "periph/pm.h"
#include "sema.h"

#include "log.h"

static firmware_update_t _state;
static ota_request_t ota_request;

static kernel_pid_t ota_update_pid = KERNEL_PID_UNDEF;
static char _msg_stack[OTA_UPDATE_STACK];
static msg_t _ota_update_msg_queue[OTA_UPDATE_MSG_QUEUE_SIZE];

#ifdef MODULE_OTA_UPDATE_TFTP
static kernel_pid_t ota_update_tftp_server_pid = KERNEL_PID_UNDEF;
static tftp_action_t _tftp_action;
/* the message queues */
#define TFTP_QUEUE_SIZE     (4)
static msg_t _tftp_msg_queue[TFTP_QUEUE_SIZE];

/* allocate the stack */
char _tftp_stack[THREAD_STACKSIZE_MAIN + THREAD_EXTRA_STACKSIZE_PRINTF + 4096];

#endif
static uint16_t remote_version = 0;
static uint16_t local_version;
static uint32_t local_appid;
static char update_name[FIRMWARE_NAME_LENGTH];

/* Counts requests sent by ota_update. */
static uint16_t req_count = 0;
static sema_t sema_fw_version = SEMA_CREATE_LOCKED();
static sema_t sema_fw_name = SEMA_CREATE_LOCKED();

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

#ifdef MODULE_OTA_UPDATE_TFTP
/* default server text which can be received */
static const char _tftp_client_hello[] = "Hello,\n"
                                         "\n"
                                         "Client text would also need to exist to be able to put data.\n"
                                         "\n"
                                         "Enjoy the RIOT-OS\n";

static int _tftp_client_data_cb(uint32_t offset, void *data, size_t data_len)
{
    return firmware_update_putbytes(&_state, offset, data, data_len);
}

/**
 * @brief called at every transaction start
 */
static bool _tftp_client_start_cb(tftp_action_t action, tftp_mode_t mode,
                                  const char *file_name, size_t *len)
{
    /* translate the mode */
    const char *str_mode = "ascii";

    if (mode == TTM_OCTET) {
        str_mode = "bin";
    }
    else if (mode == TTM_MAIL) {
        str_mode = "mail";
    }

    /* translate the action */
    const char *str_action = "read";
    if (action == TFTP_WRITE) {
        str_action = "write";
    }

    /* display the action being performed */
    printf("tftp_client: %s %s %s:%lu\n", str_mode, str_action, file_name, (unsigned long)*len);

    /* return the length of the text, if this is an read action */
    if (action == TFTP_READ) {
        *len = sizeof(_tftp_client_hello);
    }

    /* remember the action of the current transfer */
    _tftp_action = action;

    /* initialize firmware upgrade state struct */
    firmware_update_init(&_state, firmware_target_slot());

    /* we accept the transfer to take place so we return true */
    return true;
}

/**
 * @brief the transfer has stopped, see the event argument to determined if it was successful
 *        or not.
 */
static void _tftp_client_stop_cb(tftp_event_t event, const char *msg)
{
    /* decode the stop event received */
    const char *cause = "UNKOWN";
    int error = 0;

    if (event == TFTP_SUCCESS) {
        cause = "SUCCESS";
        puts("tftp_client: writing last page");
        error = firmware_update_finish(&_state);
    }
    else if (event == TFTP_PEER_ERROR) {
        cause = "ERROR From Client";
    }
    else if (event == TFTP_INTERN_ERROR) {
        cause = "ERROR Internal Server Error";
    }

    if (error != 0) {
        puts("tftp_client: Update failed!");
        printf("tftp_client: Error: %d\n", error);
        ota_update_tftp_server_start();
    }
    else {
        /* print the transfer result to the console */
        printf("tftp_client: transfer stopped: %s: %s\n", cause, (msg == NULL) ? "NULL" : msg);
    }
}

static void ota_update_tftp_client_start(uint8_t firmware_slot, char *name, char *server_ip_address)
{
    ipv6_addr_t ip;
    tftp_mode_t mode = TTM_OCTET;
    bool use_options = true;
    int ret;

    puts("tftp_client: Stopping TFTP update server...");
    ota_update_tftp_server_stop();
    puts("tftp_client: TFTP OTA update starting...");

    ipv6_addr_from_str(&ip, server_ip_address);
    _tftp_action = TFTP_READ;

    printf("tftp_client: Sending request for %s to the server %s\n",
           name,
           server_ip_address);

    ret = gnrc_tftp_client_read(&ip, name, mode, _tftp_client_data_cb,
                                _tftp_client_start_cb, _tftp_client_stop_cb,
                                use_options);

    if (ret > 0) {
        puts("tftp_client: Update finished, rebooting in 5 seconds...");
        xtimer_sleep(5);
        pm_reboot();
    } else {
        puts("tftp_client: Update failed!");
        printf("tftp_client: Error: %d\n", ret);
        ota_update_tftp_server_start();
    }
}
/**
 * @brief called at every transaction start
 */
static bool _tftp_server_start_cb(tftp_action_t action, tftp_mode_t mode,
                                  const char *file_name, size_t *len)
{
    LOG_INFO("%s: start %s %u\n", __func__, file_name, *len);

    /* make sure this is a write using octets request */
    if (mode != TTM_OCTET || action != TFTP_WRITE) {
        return false;
    }

    /* initialize firmware upgrade state struct */
    firmware_update_init(&_state, firmware_target_slot());

    /* stop the ota_update client */
    ota_update_stop();

    /* we accept the transfer to take place so we return true */
    return true;
}

/**
 * @brief called to get or put data, depending on the mode received by `_tftp_start_cb(action, ...)`
 */
static int _tftp_server_data_cb(uint32_t offset, void *data, size_t data_len)
{
    return firmware_update_putbytes(&_state, offset, data, data_len);
}

/**
 * @brief the transfer has stopped, see the event argument to determined if it was successful
 *        or not.
 */
static void _tftp_server_stop_cb(tftp_event_t event, const char *msg)
{
    /* decode the stop event received */
    const char *cause = "UNKOWN";

    if (event == TFTP_SUCCESS) {
        cause = "SUCCESS";
        firmware_update_finish(&_state);
    }
    else if (event == TFTP_PEER_ERROR) {
        cause = "ERROR From Client";
    }
    else if (event == TFTP_INTERN_ERROR) {
        cause = "ERROR Internal Server Error";
    }

    /* print the transfer result to the console */
    printf("tftp_server: transfer stopped: %s: %s\n", cause, (msg == NULL) ? "NULL" : msg);

    if (event == TFTP_SUCCESS) {
        puts("tftp_server: Update finished, rebooting in 5 seconds...");
        xtimer_sleep(5);
        pm_reboot();
    }
    else {
        ota_update_init();
    }
}

/**
 * @brief the TFTP server thread
 */
static void *tftp_server_wrapper(void *arg)
{
    (void)arg;

    /* A message queue is needed to register for incoming packets */
    msg_init_queue(_tftp_msg_queue, TFTP_QUEUE_SIZE);

    /* inform the user */
    puts("[ota_update] Starting TFTP service at port 69");

    /* run the TFTP server */
    gnrc_tftp_server(_tftp_server_data_cb, _tftp_server_start_cb, _tftp_server_stop_cb, true);

    /* the TFTP server has been stopped */
    puts("[ota_update] Server stopped");

    return NULL;
}

/**
 * @brief start the TFTP server by creating a thread
 */
kernel_pid_t ota_update_tftp_server_start(void)
{
    if (ota_update_tftp_server_pid != KERNEL_PID_UNDEF) {
        return -EEXIST;
    }

    thread_create(_tftp_stack, sizeof(_tftp_stack),
                  1, THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                  tftp_server_wrapper, NULL, "ota_tftp_server");

    return ota_update_tftp_server_pid;
}

/**
 * @brief stop the TFTP server by sending a message to the thread
 */
void ota_update_tftp_server_stop(void)
{
    gnrc_tftp_server_stop();
}
#endif /* MODULE_OTA_UPDATE_TFTP */

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
    int local_slot = firmware_current_slot();
    int target_slot = firmware_target_slot();

    memcpy(&metadata, firmware_get_metadata(local_slot), sizeof(firmware_metadata_t));

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
                sprintf(coap_resource, "/0x%lX/slot%u/name", local_appid, target_slot);
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
