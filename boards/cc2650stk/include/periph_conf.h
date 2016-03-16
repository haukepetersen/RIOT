/*
 * Copyright (C) 2016 Leon George
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_cc2650stk
 * @{
 *
 * @file
 * @brief       Peripheral MCU configuration for the CC2650STK board
 *
 * @author      Leon M. George <leon@georgemail.eu>
 */

#ifndef PERIPH_CONF_H_
#define PERIPH_CONF_H_

#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Timer configuration
 * @{
 */
static const timer_conf_t timer_config[] = {
    {
        .dev  = GPT0,
        .num  = 0
    },
    {
        .dev  = GPT1,
        .num  = 1
    }
};

#define TIMER_0_ISR         isr_timer0_chan0
#define TIMER_1_ISR         isr_timer1_chan0

#define TIMER_NUMOF         (sizeof(timer_config) / sizeof(timer_config[0]))
/** @} */


/**
 * @name UART configuration
 * @{
 */
#define UART_NUMOF          1

/* UART 0 pin configuration */
#define UART_0_RX_DIO       28
#define UART_0_TX_DIO       29
/** @} */


#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H_ */
/** @} */
