/*
 * Copyright (C) 2017 Kaspar Schleiser <kaspar@schleiser.de>
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
 * @brief       Over the air (OTA) update TFTP server
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <stdio.h>
#include <inttypes.h>
#include <errno.h>

#include "ota_tftp_server.h"
#include "thread.h"
#include "net/gnrc/tftp.h"
#include "firmware_update.h"
#include "xtimer.h"
#include "periph/pm.h"
#include "log.h"

static firmware_update_t _state;
static msg_t _tftp_msg_queue[TFTP_QUEUE_SIZE];
static kernel_pid_t ota_update_tftp_server_pid = KERNEL_PID_UNDEF;
/* allocate the stack */
char _tftp_stack[OTA_TFTP_SERVER_STACK];

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
        puts("[ota_tftp_server] Update finished, rebooting in 5 seconds...");
        xtimer_sleep(5);
        pm_reboot();
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
    puts("[ota_tftp_server] Starting TFTP service at port 69");

    /* run the TFTP server */
    gnrc_tftp_server(_tftp_server_data_cb, _tftp_server_start_cb, _tftp_server_stop_cb, true);

    /* the TFTP server has been stopped */
    puts("[ota_tftp_server] Server stopped");

    return NULL;
}

/**
 * @brief start the TFTP server by creating a thread
 */
kernel_pid_t ota_tftp_server_start(void)
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
void ota_tftp_server_stop(void)
{
    gnrc_tftp_server_stop();
}
