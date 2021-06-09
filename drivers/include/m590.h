/*
 * Copyright (C) 2014-2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_m590 M590 GSM Modem Driver
 * @ingroup     drivers
 * @brief       Device driver for M590 modems
 *
 * @{
 * @file
 * @brief       Device driver interface for M590 devices
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef DRIVER_M590_H
#define DRIVER_M590_H

#include <stdint.h>

#include "at.h"
#include "timex.h"
#include "periph/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M590_BUFSIZE            (200U)

#define M590_TIMEOUT            (250U * MS_PER_SEC)

enum {
    M590_OK      =  0,      /**< everything went as expected */
    M590_ERR     = -1,      /**< unspecified internal driver error */
    M590_NODEV   = -2,      /**< not connected to a M590 modem */
    M590_NOSIM   = -3,      /**< no SIM card present */
};

typedef struct {
    uart_t uart;
    uint32_t baudrate;
    const char *pin;
} m590_params_t;

typedef struct {
    at_dev_t at;
    char buf[M590_BUFSIZE];
    uint8_t state;
    const m590_params_t *params;
} m590_t;

int m590_init(m590_t *dev, const m590_params_t *params);

int m590_udp_open(m590_t *dev, const char *ipv4_addr, uint16_t port);
int m590_udp_close(m590_t *dev, int sock);
int m590_udp_send(m590_t *dev, int sock, const void *data, size_t data_len);

int m590_send_text(m590_t *dev, const char *recv_number, const char *text);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_M590_H */
/** @} */
