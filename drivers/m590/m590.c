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
