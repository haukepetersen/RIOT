/*
 * Copyright (C) 2013 INRIA
 *               2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup sys
 * @{
 *
 * @file
 * @brief UART stdio implementation
 *
 * This file implements a UART callback and read/write functions.
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <stdio.h>

#include "board.h"
#include "isrpipe.h"
#include "periph/uart.h"

#include "riot_stdio.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#ifndef UART_STDIO_DEV
/**
 * @brief UART device to use for STDIO
 */
#define UART_STDIO_DEV           UART_DEV(0)
#endif

#ifndef UART_STDIO_BAUDRATE
/**
 * @brief Baudrate for STDIO
 */
#define UART_STDIO_BAUDRATE      (115200)
#endif

#ifndef UART_STDIO_RX_BUFSIZE
/**
 * @brief Buffer size for STDIO
 */
#define UART_STDIO_RX_BUFSIZE    (64)
#endif

static char _rx_buf_mem[UART_STDIO_RX_BUFSIZE];
isrpipe_t uart_stdio_isrpipe = ISRPIPE_INIT(_rx_buf_mem);

void stdio_init(void)
{
    uart_init(UART_STDIO_DEV, UART_STDIO_BAUDRATE,
              (uart_rx_cb_t)isrpipe_write_one, &uart_stdio_isrpipe);
}

int stdio_read(char* buffer, int count)
{
    return isrpipe_read(&uart_stdio_isrpipe, buffer, count);
}

int stdio_write(const char* buffer, int len)
{
    uart_write(UART_STDIO_DEV, (uint8_t *)buffer, (size_t)len);
    return len;
}
