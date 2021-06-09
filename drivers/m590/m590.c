/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     drivers_m590
 * @{
 *
 * @file
 * @brief       M590 GSM modem driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "m590.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

static char _cmdbuf[16];

static char *_cmd(const char *cmd, const char *param)
{
    assert(strlen(cmd) + strlen(param) < 16);

    size_t len_cmd = strlen(cmd);
    size_t len_param = strlen(param);

    memcpy(_cmdbuf, cmd, len_cmd);
    memcpy(_cmdbuf + len_cmd, param, len_param);

    return _cmdbuf;
}

int m590_init(m590_t *dev, const m590_params_t *params)
{
    int res;
    ssize_t n;
    char cmd[16];

    dev->params = params;

    res = at_dev_init(&dev->at, params->uart, params->baudrate,
                      dev->buf, sizeof(dev->buf));
    if (res < 0) {
        return res;
    }

    /* apply SIM car PIN if applicable */
    if (params->pin != NULL) {
        sprintf(cmd, "CPIN")
        res = at_send_cmd_wait_ok(&dev->at, _cmd("AT+CPIN=", dev->pin), M590_TIMEOUT);
        if (res != 0) {
            return -1;
        }
    }

    /* check if SIM is ready */
    n = at_send_cmd_get_lines(&dev->at, "AT+CPIN?", tmp, sizeof(tmp), true, M590_TIMEOUT);
    if (n <= 0) {
        return -1;
    }
    if (memcmp(tmp, "+CPIN:READY", 11) != 0) {
        return -1;
    }

    /* check if SIM is registered with network */
    n = at_send_cmd_get_lines(&dev->at, "AT+CREG?", tmp, sizeof(tmp), true, M590_TIMEOUT);
    if (n <= 0) {
        return -1;
    }
    if (memcmp(tmp, "+CREG: 0,1", 10) != 0) {
        return -1;
    }

    /* all good then */
    return 0;

    /* check the sim status */
    /*
    AT+CPIN?        # check sim card status
    AT+CCID         # read CCID of the SIM card
    AT+CSQ          # query RSSI
    AT+CREG?        # see if module is registered to GSM network
    AT+XISP=0       # set to internal protocol
    AT+CGDCONT=1,"IP","CMNET"       # set APN
    AT+CGATT?       # Query the GPRS attach status
    AT+XIIC=1       # activate PPP connection
    AT+XIIC?        # check status of PPP connection

    AT+TCPSETUP=SOCK,IP,PORT    # setup TCP connection
    At+TCPSEND=0,10             # send data
    > datadata12                # must at least 50ms after getting `>`
    AT+IPSTATUS=0               # check connection status

    AT+TCPCLOSE=0               # close TCP connection
    AT+CGATT=0                  # close PPP connection
    */
}

int m590_udp_open(m590_t *dev, int sock, const char *ipv4_addr, uint16_t port)
{
    int res;
    char cmd[32]; // TODO: length..

    // TODO: check sock param [0-5]

    // TODO: put the following block in a if condition to do this only once...

    /* set module to internal protocol */
    res = at_send_cmd_wait_ok(&dev->at, "AT+XISP=0", M590_TIMEOUT);
    if (res != 0) {
        return -1;
    }

    /* set APN -> needed? */
    snprintf(cmd, "AT+CGDCONT=1,IP,%s", dev->params->apn)
    res = at_send_cmd_wait_ok(&dev->at, cmd, M590_TIMEOUT);
    if (res != 0) {
        return -1;
    }

    /* activate PPP connection */
    res = at_send_cmd_wait_ok(&dev->at, "AT+XIIC=1", M590_TIMEOUT);
    if (res != 0) {
        return -1;
    }

    /* setup UDP connection */
    snprintf(cmd, "AT+UDPSETUP=%i,%s,%p", sock, ipv4_addr, (unsigned)port);
    res = at_send_cmd_wait_ok(&dev->at, cmd, M590_TIMEOUT);
    if (res != 0) {
        return -1;
    }

}


int m590_init_textmode(m590_t *dev, const m590_params_t *params)
{
    int res;

    /* initialize at AT command parser */
    res = at_dev_init(&dev->at, params->uart, params->baudrate,
                      dev->buf, sizeof(dev->buf));
    if (res < 0) {
        DEBUG("[m590] err: unable to initialize AT device\n");
        return M590_ERR;
    }

    /* get IMEI to test the connection to the m590 radio */
    res = at_send_cmd_wait_ok(&dev->at, "AT+CGSN", M590_TIMEOUT);
    if (res < 0) {
        DEBUG("[m590] err: unable to read IMEI from modem\n");
        return M590_NODEV;
    }

    /* get IMI number to verify that a SIM card is present */
    res = at_send_cmd_wait_ok(&dev->at, "AT+CIMI", M590_TIMEOUT);
    if (res < 0) {
        DEBUG("[m590] err: no SIM card inserted\n");
        return M590_NOSIM;
    }

    return M590_OK;
}

int m590_send_text(m590_t *dev, const char *recv_number, const char *text)
{
    return M590_OK;
}
