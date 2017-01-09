/*
 * Copyright (C) 2014 PHYTEC Messtechnik GmbH
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_kinetis_common_uart
 *
 * @{
 *
 * @file
 * @brief       Low-level UART driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Johann Fischer <j.fischer@phytec.de>
 *
 * @}
 */

#include <math.h>

#include "cpu.h"
#include "periph_conf.h"
#include "periph/uart.h"

#ifndef KINETIS_UART_ADVANCED
/**
 * Attempts to determine the type of the UART,
 * using the BRFA field in the UART C4 register.
 */
#ifdef UART_C4_BRFA
#define KINETIS_UART_ADVANCED    1
#endif
#endif

/**
 * @brief Allocate memory to store the callback functions.
 */
static uart_isr_ctx_t ctx[UART_NUMOF];

static inline UART_Type *dev(uart_t uart)
{
    return uart_config[uart].dev;
}


int uart_init(uart_t uart, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    /* make sure given UART device is valid */
    if (uart >= UART_NUMOF) {
        return UART_NODEV;
    }

    /* disable device and reset configuration to 8N1 */
    NVIC_DisableIRQ(uart_config[uart].irqn);
    dev(uart)->C2 = 0;
    dev(uart)->C1 = 0;

    /* configure RX and TX pins */
    gpio_init_port(uart_config[uart].rx_pin, PORT_PCR_MUX(uart_config[uart].rx_af));
    gpio_init_port(uart_config[uart].tx_pin, PORT_PCR_MUX(uart_config[uart].tx_af));

    /* remember callback addresses */
    ctx[uart].rx_cb = rx_cb;
    ctx[uart].arg   = arg;

    /* calculate and set baudrate */
    uint16_t ubd = (uint16_t)(CLOCK_BUSCLOCK / (baudrate * 16));
    dev(uart)->BDH = (uint8_t)UART_BDH_SBR(ubd >> 8);
    dev(uart)->BDL = (uint8_t)UART_BDL_SBR(ubd);

#if KINETIS_UART_ADVANCED
    /* set baudrate fine adjust (brfa) */
    uint8_t brfa = ((((4 * CLOCK_BUSCLOCK) / baudrate) + 1) / 2) & 0x1f;
    dev(uart)->C4 = UART_C4_BRFA(brfa);

    /* Enable FIFO buffers */
    dev(uart)->PFIFO = (UART_PFIFO_RXFE_MASK | UART_PFIFO_TXFE_MASK);
    /* Set level to trigger TDRE flag whenever there is space in the TXFIFO */
    /* FIFO size is 2^(PFIFO_TXFIFOSIZE + 1) (4, 8, 16 ...) for values != 0.
     * TXFIFOSIZE == 0 means size = 1 (i.e. only one byte, no hardware FIFO) */
    if ((dev(uart)->PFIFO & UART_PFIFO_TXFIFOSIZE_MASK) != 0) {
        uint8_t txfifo_size =
            (2 << ((dev(uart)->PFIFO & UART_PFIFO_TXFIFOSIZE_MASK) >>
                    UART_PFIFO_TXFIFOSIZE_SHIFT));
        dev(uart)->TWFIFO = UART_TWFIFO_TXWATER(txfifo_size - 1);
    }
    else {
        /* Missing hardware support */
        dev(uart)->TWFIFO = 0;
    }
    /* trigger RX interrupt when there is 1 byte or more in the RXFIFO */
    dev(uart)->RWFIFO = 1;
    /* flush buffers, must be done whenever FIFO configuration is altered */
    dev(uart)->CFIFO = (UART_CFIFO_RXFLUSH_MASK | UART_CFIFO_TXFLUSH_MASK);
#endif

    /* enable transmitter */
    dev(uart)->C2 = (UART_C2_TE_MASK);
    /* enable receiver only if callback was given */
    if (rx_cb) {
        dev(uart)->C2 |= (UART_C2_RE_MASK | UART_C2_RIE_MASK);
        NVIC_EnableIRQ(uart_config[uart].irqn);
    }
    return UART_OK;
}

void uart_write(uart_t uart, const uint8_t *data, size_t len)
{
    assert((uart < UART_NUMOF) && data && (len > 0));

    for (size_t i = 0; i < len; i++) {
        while (!(dev(uart)->S1 & UART_S1_TDRE_MASK));
        dev(uart)->D = data[i];
    }
}

static inline void irq_handler(uart_t uart)
{
    /*
    * On Cortex-M0, it happens that S1 is read with LDR
    * instruction instead of LDRB. This will read the data register
    * at the same time and arrived byte will be lost. Maybe it's a GCC bug.
    *
    * Observed with: arm-none-eabi-gcc (4.8.3-8+..)
    * It does not happen with: arm-none-eabi-gcc (4.8.3-9+11)
    */
    if (dev(uart)->S1 & UART_S1_RDRF_MASK) {
        /* RDRF flag will be cleared when dev-D was read */
        ctx[uart].rx_cb(ctx[uart].arg, (uint8_t)dev(uart)->D);
    }

#if (KINETIS_UART_ADVANCED == 0)
    /* clear overrun flag if set */
    if (dev(uart)->S1 & UART_S1_OR_MASK) {
        dev(uart)->S1 = UART_S1_OR_MASK;
    }
#endif

    cortexm_isr_end();
}

#ifdef UART_0_EN
void UART_0_ISR(void)
{
    irq_handler(UART_DEV(0));
}
#endif

#ifdef UART_1_EN
void UART_1_ISR(void)
{
    irq_handler(UART_DEV(1));
}
#endif
