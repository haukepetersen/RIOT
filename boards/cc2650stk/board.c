/*
 * Copyright (C) 2016 Leon George
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */
/**
 * @ingroup     boards_cc2650stk
 * @{
 *
 * @file
 * @brief       Board specific implementations for the CC2650STK board
 *
 * @author      Leon M. George <leon@georgemail.eu>
 */

#include "board.h"
#include "cpu_conf.h"

#include "periph/uart.h"

void board_init(void)
{
    cpu_init();

    gpio_init(LED_RED_DIO, GPIO_DIR_OUT, GPIO_NOPULL);
    gpio_init(LED_GREEN_DIO, GPIO_DIR_OUT, GPIO_NOPULL);
}

/** @} */
