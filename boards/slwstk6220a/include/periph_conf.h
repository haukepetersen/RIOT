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
 * @brief       Configuration of CPU peripherals for the WSTK6220 board
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include <stdint.h>
#include "cpu.h"
#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Clock configuration
 * @{
 */
#define CLOCK_CORECLOCK     (14000000U)     /* run @ 48MHz */

#define CLOCK_HFXO          (48000000U)     /* external HF crystal speed */
#define CLOCK_HFPERCLK      CLOCK_CORECLOCK
/** @} */

/**
 * @brief   Timer configuration
 * @{
 */
static const timer_conf_t timer_config[] = {
    {
        TIMER0,             /* lower numbered timer, used as pre-scaler */
        TIMER1,             /* higher numbered timer, this is the one */
        5,                  /* pre-scaler bit in the CMU register */
        TIMER1_IRQn,        /* IRQn of the higher numbered driver */
    }
};

#define TIMER_0_ISR         isr_timer1
#define TIMER_0_MAX_VALUE   (0xffff)            /* 16-bit timer */
#define TIMER_NUMOF         (sizeof(timer_config) / sizeof(timer_config[0]))
/** @} */

/**
 * @brief   UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
    {
        USART2,             /* device */
        GPIO_DEV(PB,4),     /* RX pin */
        GPIO_DEV(PB,3),     /* TX pin */
        1,                  /* AF location */
        2,                  /* bit in CMU enable register */
        USART2_RX_IRQn      /* IRQ base channel */
    },
};

#define UART_0_ISR_TX       isr_usart2_tx
#define UART_0_ISR_RX       isr_usart2_rx
#define UART_NUMOF          (sizeof(uart_config) / sizeof(uart_config[0]))
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */
