/*
 * Copyright (C) 2014-2017 Freie Universität Berlin
 * Copyright (C) 2016 OTA keys
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_cortexm_common
 * @ingroup     drivers_periph_uart
 * @{
 *
 * @file
 * @brief       Low-level UART driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Fabian Nack <nack@inf.fu-berlin.de>
 * @author      Hermann Lelong <hermann@otakeys.com>
 * @author      Toon Stegen <toon.stegen@altran.com>
 *
 * @}
 */

#include "cpu.h"
#include "sched.h"
#include "thread.h"
#include "assert.h"
#include "periph/uart2.h"
#include "periph/gpio.h"

#ifdef UART_NUMOF

#define RXENABLE            (USART_CR1_RE | USART_CR1_RXNEIE)

/**
 * @brief   Allocate memory to store the callback functions
 */
static uart_isr_ctx_t isr_ctx[UART_NUMOF];

static inline USART_TypeDef *dev(unsigned num)
{
    return uart_config[num].dev;
}

static int init(const void *cfg, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    uint16_t mantissa;
    uint8_t fraction;
    uint32_t clk;

    unsigned num = (unsigned)cfg;
    assert(num < UART_NUMOF);

    /* save ISR context */
    isr_ctx[num].rx_cb = rx_cb;
    isr_ctx[num].arg   = arg;

    /* configure TX pin */
    gpio_init(uart_config[num].tx_pin, GPIO_OUT);
    /* set TX pin high to avoid garbage during further initialization */
    gpio_set(uart_config[num].tx_pin);
#ifdef CPU_FAM_STM32F1
    gpio_init_af(uart_config[num].tx_pin, GPIO_AF_OUT_PP);
#else
    gpio_init_af(uart_config[num].tx_pin, uart_config[num].tx_af);
#endif
    /* configure RX pin */
    if (rx_cb) {
        gpio_init(uart_config[num].rx_pin, GPIO_IN);
#ifndef CPU_FAM_STM32F1
        gpio_init_af(uart_config[num].rx_pin, uart_config[num].rx_af);
#endif
    }
#ifdef UART_USE_HW_FC
    if (uart_config[num].cts_pin != GPIO_UNDEF) {
        gpio_init(uart_config[num].cts_pin, GPIO_IN);
        gpio_init(uart_config[num].rts_pin, GPIO_OUT);
#ifdef CPU_FAM_STM32F1
        gpio_init_af(uart_config[num].rts_pin, GPIO_AF_OUT_PP);
#else
        gpio_init_af(uart_config[num].cts_pin, uart_config[num].cts_af);
        gpio_init_af(uart_config[num].rts_pin, uart_config[num].rts_af);
#endif
    }
#endif

    /* enable the clock */
    periph_clk_en(uart_config[num].bus, uart_config[num].rcc_mask);

    /* reset UART configuration -> defaults to 8N1 mode */
    dev(num)->CR1 = 0;
    dev(num)->CR2 = 0;
    dev(num)->CR3 = 0;

    /* calculate and apply baudrate */
    clk = periph_apb_clk(uart_config[num].bus) / baudrate;
    mantissa = (uint16_t)(clk / 16);
    fraction = (uint8_t)(clk - (mantissa * 16));
    dev(num)->BRR = ((mantissa & 0x0fff) << 4) | (fraction & 0x0f);

    /* enable RX interrupt if applicable */
    if (rx_cb) {
        NVIC_EnableIRQ(uart_config[num].irqn);
        dev(num)->CR1 = (USART_CR1_UE | USART_CR1_TE | RXENABLE);
    }
    else {
        dev(num)->CR1 = (USART_CR1_UE | USART_CR1_TE);
    }

#ifdef UART_USE_HW_FC
    if (uart_config[num].cts_pin != GPIO_UNDEF) {
        /* configure hardware flow control */
        dev(num)->CR3 = (USART_CR3_RTSE | USART_CR3_CTSE);
    }
#endif

    return UART_OK;
}

static void write(const void *cfg, const uint8_t *data, size_t len)
{
    unsigned num = (unsigned)cfg;
    assert(num < UART_NUMOF);

    for (size_t i = 0; i < len; i++) {
#if defined(CPU_FAM_STM32F0) || defined(CPU_FAM_STM32L0) \
    || defined(CPU_FAM_STM32F3) || defined(CPU_FAM_STM32L4) \
    || defined(CPU_FAM_STM32F7)
        while (!(dev(num)->ISR & USART_ISR_TXE)) {}
        dev(num)->TDR = data[i];
#else
        while (!(dev(num)->SR & USART_SR_TXE)) {}
        dev(num)->DR = data[i];
#endif
    }

    /* make sure the function is synchronous by waiting for the transfer to
     * finish */
#if defined(CPU_FAM_STM32F0) || defined(CPU_FAM_STM32L0) \
    || defined(CPU_FAM_STM32F3) || defined(CPU_FAM_STM32L4) \
    || defined(CPU_FAM_STM32F7)
    while (!(dev(num)->ISR & USART_ISR_TC)) {}
#else
    while (!(dev(num)->SR & USART_SR_TC)) {}
#endif
}

static void poweron(const void *cfg)
{
    unsigned num = (unsigned)cfg;
    assert(num < UART_NUMOF);
    periph_clk_en(uart_config[num].bus, uart_config[num].rcc_mask);
}

static void poweroff(const void *cfg)
{
    unsigned num = (unsigned)cfg;
    assert(num < UART_NUMOF);
    periph_clk_en(uart_config[num].bus, uart_config[num].rcc_mask);
}

const uart_api_t stm32_uart = {
    .init     = init,
    .write    = write,
    .poweron  = poweron,
    .poweroff = poweroff
};

static inline void irq_handler(unsigned num)
{
#if defined(CPU_FAM_STM32F0) || defined(CPU_FAM_STM32L0) \
    || defined(CPU_FAM_STM32F3) || defined(CPU_FAM_STM32L4) \
    || defined(CPU_FAM_STM32F7)

    uint32_t status = dev(num)->ISR;

    if (status & USART_ISR_RXNE) {
        isr_ctx[num].rx_cb(isr_ctx[num].arg, (uint8_t)dev(num)->RDR);
    }
    if (status & USART_ISR_ORE) {
        dev(num)->ICR |= USART_ICR_ORECF;    /* simply clear flag on overrun */
    }

#else

    uint32_t status = dev(num)->SR;

    if (status & USART_SR_RXNE) {
        isr_ctx[num].rx_cb(isr_ctx[num].arg, (uint8_t)dev(num)->DR);
    }
    if (status & USART_SR_ORE) {
        /* ORE is cleared by reading SR and DR sequentially */
        dev(num)->DR;
    }

#endif

    cortexm_isr_end();
}

#ifdef UART_0_ISR
void UART_0_ISR(void)
{
    irq_handler(0);
}
#endif

#ifdef UART_1_ISR
void UART_1_ISR(void)
{
    irq_handler(1);
}
#endif

#ifdef UART_2_ISR
void UART_2_ISR(void)
{
    irq_handler(2);
}
#endif

#ifdef UART_3_ISR
void UART_3_ISR(void)
{
    irq_handler(3);
}
#endif

#ifdef UART_4_ISR
void UART_4_ISR(void)
{
    irq_handler(4);
}
#endif

#ifdef UART_5_ISR
void UART_5_ISR(void)
{
    irq_handler(5);
}
#endif

#endif /* UART_NUMOF */
