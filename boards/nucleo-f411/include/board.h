/*
 * Copyright (C) 2016 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    boards_nucleo-f411 Nucleo-F411
 * @ingroup     boards_nucleo
 * @brief       Board specific files for the nucleo-f411 board
 * @{
 *
 * @file
 * @brief       Board specific definitions for the nucleo-f411 board
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef BOARD_H
#define BOARD_H

#include "board_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    xtimer configuration
 * @{
 */
#define XTIMER_OVERHEAD     (6)
#define XTIMER_BACKOFF      (5)
/** @} */

#include "periph_conf.h"
#include "periph/uart2.h"
// #include "periph_stm32.h"

#define UART_PERIPH_NUMOF   (sizeof(uart_periph) / sizeof(uart_periph[0]))
#define UART_PERIPH(x)      (&uart_periph[x])

extern const uart_api_t stm32_uart;

static const uart_t uart_periph[] = {
    // { .api = &stm32_lpuart, .cfg = &lpuart_config[0] },
    { .api = &stm32_uart, .cfg = (void *)0 },
    { .api = &stm32_uart, .cfg = (void *)1 }
};

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
