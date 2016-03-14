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
 * @brief       peripheral MCU configuration for the CC2650STK board
 *
 * @author      Leon M. George <leon@georgemail.eu>
 */

#ifndef PERIPH_CONF_H_
#define PERIPH_CONF_H_

#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XTIMER_MASK                 (0xFFFF0000)

/**
 * @name timer peripheral configuration
 * @{
 */
#define TIMER_NUMOF         (sizeof(timer_conf) / sizeof(timer_conf[0]))

#define TIMER_0_IRQn_1      GPTIMER_0A_IRQN
#define TIMER_0_IRQn_2      GPTIMER_0B_IRQN
// TODO won't compile without; can't remove. don't see how
void isr_timer0_chan0(void);
void isr_timer0_chan1(void);
#define TIMER_0_ISR_1       isr_timer0_chan0
#define TIMER_0_ISR_2       isr_timer0_chan1

#define TIMER_1_IRQn_1      GPTIMER_1A_IRQN
#define TIMER_1_IRQn_2      GPTIMER_1B_IRQN
void isr_timer1_chan0(void);
void isr_timer1_chan1(void);
#define TIMER_1_ISR_1       isr_timer1_chan0
#define TIMER_1_ISR_2       isr_timer1_chan1

#define TIMER_2_IRQn_1      GPTIMER_2A_IRQN
#define TIMER_2_IRQn_2      GPTIMER_2B_IRQN
void isr_timer2_chan0(void);
void isr_timer2_chan1(void);
#define TIMER_2_ISR_1       isr_timer2_chan0
#define TIMER_2_ISR_2       isr_timer2_chan1

#define TIMER_3_IRQn_1      GPTIMER_3A_IRQN
#define TIMER_3_IRQn_2      GPTIMER_3B_IRQN
void isr_timer3_chan0(void);
void isr_timer3_chan1(void);
#define TIMER_3_ISR_1       isr_timer3_chan0
#define TIMER_3_ISR_2       isr_timer3_chan1

static const timer_conf_t timer_conf[] = {
    {
        GPT0,
        TIMER_0_IRQn_1,
        TIMER_0_ISR_1,
    },
    {
        GPT1,
        TIMER_1_IRQn_1,
        TIMER_1_ISR_1,
    },
    {
        GPT2,
        TIMER_2_IRQn_1,
        TIMER_2_ISR_1,
    },
    {
        GPT3,
        TIMER_3_IRQn_1,
        TIMER_3_ISR_1,
    },
};
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

#define CCFG_BACKDOOR_ENABLE       0 /* impossible to use due to the board design */

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* PERIPH_CONF_H_ */
/** @} */
