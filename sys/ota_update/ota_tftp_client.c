/*
 * Copyright (C) 2017 Inria
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
 * @brief       Over the air (OTA) updates TFTP client
 *
 * @author      Francisco Acosta <francisco.acosta@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <inttypes.h>

#include "net/gnrc/tftp.h"
#include "firmware_update.h"
#include "ota_update.h"
#include "xtimer.h"
#include "periph/pm.h"

static firmware_update_t _state;
static tftp_action_t _tftp_action;

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
    printf("[ota_tftp_client] %s %s %s:%lu\n", str_mode, str_action, file_name, (unsigned long)*len);

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

    if (event == TFTP_SUCCESS) {
        cause = "SUCCESS";
        puts("[ota_tftp_client] writing last page");
        firmware_update_finish(&_state);
    }
    else if (event == TFTP_PEER_ERROR) {
        cause = "ERROR From Client";
    }
    else if (event == TFTP_INTERN_ERROR) {
        cause = "ERROR Internal Server Error";
    }

    /* print the transfer result to the console */
    printf("[ota_tftp_client] transfer stopped: %s: %s\n", cause, (msg == NULL) ? "NULL" : msg);
}

void ota_update_tftp_client_start(uint8_t firmware_slot, char *name, char *server_ip_address)
{
    ipv6_addr_t ip;
    tftp_mode_t mode = TTM_OCTET;
    bool use_options = true;
    int ret;

    ipv6_addr_from_str(&ip, server_ip_address);
    _tftp_action = TFTP_READ;

    printf("[ota_tftp_client] Sending request for %s to the server %s\n",
           name,
           server_ip_address);

    ret = gnrc_tftp_client_read(&ip, name, mode, _tftp_client_data_cb,
                                _tftp_client_start_cb, _tftp_client_stop_cb,
                                use_options);

    if (ret > 0) {
        puts("[ota_tftp_client] Update finished, rebooting in 5 seconds...");
        xtimer_sleep(5);
        pm_reboot();
    } else {
        puts("[ota_tftp_client] Update failed!");
        printf("[ota_tftp_client] Error: %d\n", ret);
    }
}
