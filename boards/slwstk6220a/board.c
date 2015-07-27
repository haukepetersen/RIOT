/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_wstk6220
 * @{
 *
 * @file
 * @brief       Board specific implementations WSTK6220 board
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "board.h"
#include "cpu.h"
#include "periph/gpio.h"

// #include <stdio.h>
// #include "periph/uart.h"

void board_init(void)
{
    /* initialize the CPU */
    cpu_init();

    /* enable access to the evaluation board controller chip */
    gpio_init(BC_PIN, GPIO_DIR_OUT, GPIO_NOPULL);
    gpio_set(BC_PIN);

    /* initialize the boards LEDs */
    gpio_init(LED0_PIN, GPIO_DIR_OUT, GPIO_NOPULL);
    gpio_init(LED1_PIN, GPIO_DIR_OUT, GPIO_NOPULL);

    // uart_init(STDIO, 115200, NULL, NULL, NULL);
    // uart_write_blocking(STDIO, 'A');
    // uart_write_blocking(STDIO, 'B');
    // uart_write_blocking(STDIO, 'C');
    // uart_write_blocking(STDIO, '\n');
}
