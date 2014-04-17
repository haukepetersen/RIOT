/*
 * Copyright (C) 2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     board_stm32f4discovery
 * @{
 *
 * @file        board.c
 * @brief       Board specific implementations for the STM32F4Discovery evaluation board
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "board.h"

#include "periph/uart.h"

extern void SystemInit(void);
void leds_init(void);


void board_init(void)
{
    /* initialize core clocks via STM-lib given function */
    SystemInit();

    /* initialize the CPU */
    cpu_init();

    /* initialize the boards LEDs */
    leds_init();

    // LD3_ON;
    // LD5_ON;
    // LD4_TOGGLE;

    // uart_init_blocking(UART_0, 115200);

    // while (1) {
    //     for (int i = 10000000; i > 0; i--);
    //     LD3_TOGGLE;
    //     LD4_TOGGLE;
    //     LD5_TOGGLE;
    //     LD6_TOGGLE;
    //     uart_write_blocking(UART_0, 'A');
    //     uart_write_blocking(UART_0, '\n');
    // }
}


/**
 * @brief Initialize the boards on-board LEDs (LD3 and LD4)
 * 
 * The LED initialization is hard-coded in this function. As the LEDs are soldered
 * onto the board they are fixed to their CPU pins.
 * 
 * The LEDs are connected to the following pins:
 * - LD3: PD13
 * - LD4: PD12
 * - LD5: PD14
 * - LD6: PD15
 */
void leds_init(void)
{
    /* enable clock for port GPIOD */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    /* configure pins as general outputs */
    LED_PORT->MODER |= 0x55000000;
    /* set output speed high-speed */
    LED_PORT->OSPEEDR |= 0xff000000;
    /* set output type to push-pull */
    LED_PORT->OTYPER &= ~(0x0000f000);
    /* disable pull resistors */
    LED_PORT->PUPDR &= ~(0xff000000);

    /* turn all LEDs off */
    LED_PORT->BSRRH |= 0xf000;
}
