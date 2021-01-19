/*
 * Copyright (C) 2017 Freie Universität Berlin
 *               2017 Inria
 *               2017 HAW-Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_nucleo-l476rg
 * @{
 *
 * @file
 * @brief       Peripheral MCU configuration for the nucleo-l476rg board
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @author      Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 */

#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

/* Add specific clock configuration (HSE, LSE) for this board here */
#ifndef CONFIG_BOARD_HAS_LSE
#define CONFIG_BOARD_HAS_LSE            1
#endif

#include "periph_cpu.h"
#include "clk_conf.h"
#include "cfg_i2c1_pb8_pb9.h"
#include "cfg_rtt_default.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    DMA streams configuration
 * @{
 */
static const dma_conf_t dma_config[] = {
    { .stream = 1 },    /* DMA1 Channel 2 - SPI1_RX | USART3_TX */
    { .stream = 2 },    /* DMA1 Channel 3 - SPI1_TX */
    { .stream = 3 },    /* DMA1 Channel 4 - USART1_TX */
    { .stream = 6 },    /* DMA1 Channel 7 - USART2_TX */
};

#define DMA_0_ISR  isr_dma1_channel2
#define DMA_1_ISR  isr_dma1_channel3
#define DMA_2_ISR  isr_dma1_channel4
#define DMA_3_ISR  isr_dma1_channel7

#define DMA_NUMOF           ARRAY_SIZE(dma_config)
/** @} */


/**
 * @name    Timer configuration
 * @{
 */
static const timer_conf_t timer_config[] = {
    {
        .dev      = TIM2,
        .max      = 0xffffffff,
        .rcc_mask = RCC_APB1ENR1_TIM2EN,
        .bus      = APB1,
        .irqn     = TIM2_IRQn
    }
};

#define TIMER_0_ISR         isr_tim5

#define TIMER_NUMOF         ARRAY_SIZE(timer_config)
/** @} */

/**
 * @name    UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
    {
        .dev        = USART1,
        .rcc_mask   = RCC_APB2ENR_USART1EN,
        .rx_pin     = GPIO_PIN(PORT_A, 3),
        .tx_pin     = GPIO_PIN(PORT_A, 2),
        .rx_af      = GPIO_AF7,
        .tx_af      = GPIO_AF7,
        .bus        = APB2,
        .irqn       = USART1_IRQn,
        // .type       = STM32_USART,
        // .clk_src    = 0, /* Use APB clock */
#ifdef MODULE_PERIPH_DMA
        .dma        = 3,
        .dma_chan   = 2
#endif
    },
};

#define UART_0_ISR          (isr_usart2)
#define UART_1_ISR          (isr_usart3)
#define UART_2_ISR          (isr_usart1)

#define UART_NUMOF          ARRAY_SIZE(uart_config)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */
