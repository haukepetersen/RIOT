/*
 * Copyright (C) 2016  OTA keys S.A.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_nucleo-f207
 * @{
 *
 * @file
 * @name        Peripheral MCU configuration for the nucleo-f207 board
 *
 * @author      Vincent Dupont <vincent@otakeys.com>
 * @author      Aurelien Gonce <aurelien.gonce@altran.fr>
 * @author      Toon Stegen <toon.stegen@altran.com>
 */

#ifndef PERIPH_CONF_H_
#define PERIPH_CONF_H_

#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Clock system configuration
 * @{
 */
#define CLOCK_HSE           (8000000U)          /* external oscillator */
#define CLOCK_CORECLOCK     (120000000U)        /* desired core clock frequency */

/* the actual PLL values are automatically generated */
#define CLOCK_PLL_M         (CLOCK_HSE / 1000000)
#define CLOCK_PLL_N         ((CLOCK_CORECLOCK / 1000000) * 2)
#define CLOCK_PLL_P         (2U)
#define CLOCK_PLL_Q         (CLOCK_PLL_N / 48)
#define CLOCK_AHB_DIV       RCC_CFGR_HPRE_DIV1
#define CLOCK_APB1_DIV      RCC_CFGR_PPRE1_DIV4
#define CLOCK_APB2_DIV      RCC_CFGR_PPRE2_DIV2
#define CLOCK_FLASH_LATENCY FLASH_ACR_LATENCY_5WS

/* bus clocks for simplified peripheral initialization, UPDATE MANUALLY! */
#define CLOCK_AHB           (CLOCK_CORECLOCK / 1)
#define CLOCK_APB1          (CLOCK_CORECLOCK / 4)
#define CLOCK_APB2          (CLOCK_CORECLOCK / 2)
/** @} */

/**
 * @name PWM configuration
 * @{
 */
#define PWM_NUMOF         (1U)
#define PWM_0_EN          1

static const pwm_conf_t pwm_config[PWM_NUMOF] = {
    {
        .tim      = 2,
        .port     = GPIOC,
        .bus      = AHB1,
        .rcc_mask = RCC_AHB1ENR_GPIOCEN,
        .CH0      = 6,
        .CH1      = 7,
        .CH2      = 8,
        .CH3      = 9,
        .AF       = 2
    }
};
/** @} */

/**
 * @name Timer configuration
 * @{
 */
#define TIMER_NUMOF         (4U)
#define TIMER_0_EN          1
#define TIMER_1_EN          1
#define TIMER_2_EN          1
#define TIMER_3_EN          1
#define TIMER_IRQ_PRIO      1

static const timer_conf_t timer_config[TIMER_NUMOF] = {
    {
        .dev      = TIM2,
        .channels = 4,
        .freq     = (CLOCK_APB1 * 2),
        .rcc_mask = RCC_APB1ENR_TIM2EN,
        .bus      = APB1,
        .irqn     = TIM2_IRQn,
        .priority = TIMER_IRQ_PRIO
    },
    {
        .dev      = TIM5,
        .channels = 4,
        .freq     = (CLOCK_APB1 * 2),
        .rcc_mask = RCC_APB1ENR_TIM5EN,
        .bus      = APB1,
        .irqn     = TIM5_IRQn,
        .priority = TIMER_IRQ_PRIO
    },
    {
        .dev      = TIM3,
        .channels = 4,
        .freq     = (CLOCK_APB1 * 2),
        .rcc_mask = RCC_APB1ENR_TIM3EN,
        .bus      = APB1,
        .irqn     = TIM3_IRQn,
        .priority = TIMER_IRQ_PRIO
    },
    {
        .dev      = TIM4,
        .channels = 4,
        .freq     = (CLOCK_APB1 * 2),
        .rcc_mask = RCC_APB1ENR_TIM4EN,
        .bus      = APB1,
        .irqn     = TIM4_IRQn,
        .priority = TIMER_IRQ_PRIO
    }
};

#define TIMER_0_ISR         isr_tim2
#define TIMER_1_ISR         isr_tim5
#define TIMER_2_ISR         isr_tim3
#define TIMER_3_ISR         isr_tim4
/** @} */

/**
 * @brief   UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
     {
        .dev          = USART3,
        .rcc_mask     = RCC_APB1ENR_USART3EN,
        .rx_pin       = GPIO_PIN(PORT_D, 9),
        .tx_pin       = GPIO_PIN(PORT_D, 8),
        .rx_mode      = GPIO_IN,
        .tx_mode      = GPIO_OUT,
        .rts_pin      = GPIO_PIN(PORT_D, 12),
        .cts_pin      = GPIO_PIN(PORT_D, 11),
        .rts_mode     = GPIO_OUT,
        .cts_mode     = GPIO_IN,
        .af           = GPIO_AF7,
        .irqn         = USART3_IRQn,
        .dma_stream   = 3,
        .dma_chan     = 4,
        .hw_flow_ctrl = 0
    },
    {
        .dev          = USART2,
        .rcc_mask     = RCC_APB1ENR_USART2EN,
        .rx_pin       = GPIO_PIN(PORT_D, 6),
        .tx_pin       = GPIO_PIN(PORT_D, 5),
        .rx_mode      = GPIO_IN,
        .tx_mode      = GPIO_OUT,
        .rts_pin      = GPIO_PIN(PORT_D, 4),
        .cts_pin      = GPIO_PIN(PORT_D, 3),
        .rts_mode     = GPIO_OUT,
        .cts_mode     = GPIO_IN,
        .af           = GPIO_AF7,
        .irqn         = USART2_IRQn,
        .dma_stream   = 6,
        .dma_chan     = 4,
        .hw_flow_ctrl = 1
    },
    {
        .dev          = USART1,
        .rcc_mask     = RCC_APB2ENR_USART1EN,
        .rx_pin       = GPIO_PIN(PORT_A, 10),
        .tx_pin       = GPIO_PIN(PORT_A, 9),
        .rx_mode      = GPIO_IN,
        .tx_mode      = GPIO_OUT,
        .rts_pin      = GPIO_PIN(PORT_A, 12),
        .cts_pin      = GPIO_PIN(PORT_A, 11),
        .rts_mode     = GPIO_OUT,
        .cts_mode     = GPIO_IN,
        .af           = GPIO_AF7,
        .irqn         = USART1_IRQn,
        .dma_stream   = 7,
        .dma_chan     = 4,
        .hw_flow_ctrl = 1
    }
};

/* assign ISR vector names */
#define UART_0_ISR          isr_usart3
#define UART_0_DMA_ISR      isr_dma1_stream3

#define UART_1_ISR          isr_usart2
#define UART_1_DMA_ISR      isr_dma1_stream6

#define UART_2_ISR          isr_usart1
#define UART_2_DMA_ISR      isr_dma1_stream7

/* deduct number of defined UART interfaces */
#define UART_NUMOF          (sizeof(uart_config) / sizeof(uart_config[0]))
/** @} */

/**
 * @name SPI configuration
 * @{
 */
static const spi_conf_t spi_config[] = {
    {
        .dev      = SPI1,
        .mosi_pin = GPIO_PIN(PORT_A, 7),
        .miso_pin = GPIO_PIN(PORT_A, 6),
        .sclk_pin = GPIO_PIN(PORT_A, 5),
        .cs_pin   = GPIO_PIN(PORT_A, 4),
        .af       = 5,
        .abpbus   = APB2,
        .rccmask  = RCC_APB2ENR_SPI1EN,
    },
    {
        .dev      = SPI2,
        .mosi_pin = GPIO_PIN(PORT_B, 5),
        .miso_pin = GPIO_PIN(PORT_B, 4),
        .sclk_pin = GPIO_PIN(PORT_B, 3),
        .cs_pin   = GPIO_PIN(PORT_B, 2),
        .af       = 5,
        .abpbus   = APB1,
        .rccmask  = RCC_APB1ENR_SPI2EN,
    }
};

#define SPI_NUMOF               (sizeof(spi_config) / sizeof(spi_config[0]))

/** @} */

/**
 * @name I2C configuration
 * @{
 */
#define I2C_NUMOF           (1U)
#define I2C_0_EN            1
#define I2C_IRQ_PRIO        1
#define I2C_APBCLK          (CLOCK_APB1)

/* I2C 0 device configuration */
#define I2C_0_DEV           I2C1
#define I2C_0_CLKEN()       (RCC->APB1ENR |= RCC_APB1ENR_I2C1EN)
#define I2C_0_CLKDIS()      (RCC->APB1ENR &= ~(RCC_APB1ENR_I2C1EN))
#define I2C_0_EVT_IRQ       I2C1_EV_IRQn
#define I2C_0_EVT_ISR       isr_i2c1_ev
#define I2C_0_ERR_IRQ       I2C1_ER_IRQn
#define I2C_0_ERR_ISR       isr_i2c1_er
/* I2C 0 pin configuration */
#define I2C_0_SCL_PORT      GPIOB
#define I2C_0_SCL_PIN       8
#define I2C_0_SCL_AF        4
#define I2C_0_SCL_PULLUP    0
#define I2C_0_SCL_CLKEN()   (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN)
#define I2C_0_SDA_PORT      GPIOB
#define I2C_0_SDA_PIN       9
#define I2C_0_SDA_AF        4
#define I2C_0_SDA_PULLUP    0
#define I2C_0_SDA_CLKEN()   (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN)
/** @} */

/**
 * @brief   ADC configuration
 *
 * We need to define the following fields:
 * PIN, device (ADCx), channel
 * @{
 */
#define ADC_CONFIG {              \
    {GPIO_PIN(PORT_A, 4), 0, 0},  \
    {GPIO_PIN(PORT_A, 5), 1, 0}  \
}
#define ADC_NUMOF          (2)

/** @} */

/**
 * @brief   DAC configuration
 * @{
 */
#define DAC_NUMOF           (0)
/** @} */

/**
 * @brief   RTC configuration
 * @{
 */
#define RTC_NUMOF           (1)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H_ */
/** @} */
