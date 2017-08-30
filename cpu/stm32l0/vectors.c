/*
 * Copyright (C) 2014-2017 Freie Universität Berlin
 *               2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_stm32l0
 * @{
 *
 * @file
 * @brief       Interrupt vector definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdint.h>
#include "vectors_cortexm.h"

/* get the start of the ISR stack as defined in the linkerscript */
extern uint32_t _estack;

/* define a local dummy handler as it needs to be in the same compilation unit
 * as the alias definition */
void dummy_handler(void) {
    dummy_handler_default();
}

/* STM32L0 specific interrupt vectors */
WEAK_DEFAULT void isr_wwdg(void);
WEAK_DEFAULT void isr_pvd(void);
WEAK_DEFAULT void isr_rtc(void);
WEAK_DEFAULT void isr_flash(void);
WEAK_DEFAULT void isr_rcc(void);
WEAK_DEFAULT void isr_exti(void);
WEAK_DEFAULT void isr_ts(void);
WEAK_DEFAULT void isr_dma1_ch1(void);
WEAK_DEFAULT void isr_dma1_ch2_3(void);
WEAK_DEFAULT void isr_dma1_ch4_5_6_7(void);
WEAK_DEFAULT void isr_adc1_comp(void);
WEAK_DEFAULT void isr_lptim1(void);
WEAK_DEFAULT void isr_usart4_5(void);
WEAK_DEFAULT void isr_tim2(void);
WEAK_DEFAULT void isr_tim3(void);
WEAK_DEFAULT void isr_tim6_dac(void);
WEAK_DEFAULT void isr_tim7(void);
WEAK_DEFAULT void isr_tim21(void);
WEAK_DEFAULT void isr_i2c3(void);
WEAK_DEFAULT void isr_tim22(void);
WEAK_DEFAULT void isr_i2c1(void);
WEAK_DEFAULT void isr_i2c2(void);
WEAK_DEFAULT void isr_spi1(void);
WEAK_DEFAULT void isr_spi2(void);
WEAK_DEFAULT void isr_usart1(void);
WEAK_DEFAULT void isr_usart2(void);
WEAK_DEFAULT void isr_rng_lpuart1(void);
WEAK_DEFAULT void isr_lpuart1(void);
WEAK_DEFAULT void isr_lcd(void);
WEAK_DEFAULT void isr_usb(void);

/* CPU specific interrupt vector table */
ISR_VECTOR(1) const isr_t vector_cpu[CPU_IRQ_NUMOF] = {
    [0] = isr_wwdg,               /* [0] windowed watchdog */
    [1] = isr_pvd,                /* [1] power control */
    [2] = isr_rtc,                /* [2] real time clock */
    [3] = isr_flash,              /* [3] flash memory controller */
    [4] = isr_rcc,                /* [4] reset and clock control */
    [5] = isr_exti,               /* [5] external interrupt lines 0 and 1 */
    [6] = isr_exti,               /* [6] external interrupt lines 2 and 3 */
    [7] = isr_exti,               /* [7] external interrupt lines 4 to 15 */

#if defined(CPU_MODEL_STM32L053R8) || defined(CPU_MODEL_STM32L073RZ) || \
    defined(CPU_MODEL_STM32L072CZ)
    [8]  = isr_ts,                 /* [8] touch sensing input*/
#endif

    [9]  = isr_dma1_ch1,           /* [9] direct memory access controller 1, channel 1*/
    [10] = isr_dma1_ch2_3,         /* [10] direct memory access controller 1, channel 2 and 3*/
    [11] = isr_dma1_ch4_5_6_7,     /* [11] direct memory access controller 1, channel 4, 5, 6 and 7*/
    [12] = isr_adc1_comp,          /* [12] analog digital converter */
    [13] = isr_lptim1,             /* [13] low power timer 1 */

#if defined(CPU_MODEL_STM32L073RZ) || defined(CPU_MODEL_STM32L072CZ)
    [14] = isr_usart4_5,           /* [14] usart 4 to 5 */
#endif

    [15] = isr_tim2,               /* [15] timer 2 */

#if defined(CPU_MODEL_STM32L073RZ) || defined(CPU_MODEL_STM32L072CZ)
    [16] = isr_tim3,               /* [16] timer 3 */
#endif

#if defined(CPU_MODEL_STM32L053R8) || defined(CPU_MODEL_STM32L073RZ) || \
    defined(CPU_MODEL_STM32L072CZ)
    [17] = isr_tim6_dac,           /* [17] timer 6 and digital to analog converter */
#endif

#if defined(CPU_MODEL_STM32L073RZ) || defined(CPU_MODEL_STM32L072CZ)
    [18] = isr_tim7,               /* [18] timer 7 */
#endif

    [20] = isr_tim21,              /* [20] timer 21 */

#if defined(CPU_MODEL_STM32L073RZ) || defined(CPU_MODEL_STM32L072CZ)
    [21] = isr_i2c3,               /* [21] I2C 3 */
#endif

    [22] = isr_tim22,              /* [22] timer 22 */
    [23] = isr_i2c1,               /* [23] I2C 1 */

#if defined(CPU_MODEL_STM32L053R8)
    [24] = isr_i2c2,               /* [24] I2C 2 */
#endif

    [25] = isr_spi1,               /* [25] SPI 1 */

#if defined(CPU_MODEL_STM32L053R8) || defined(CPU_MODEL_STM32L073RZ) || \
    defined(CPU_MODEL_STM32L072CZ)
    [26] = isr_spi2,               /* [26] SPI 2 */
    [27] = isr_usart1,             /* [27] USART 1 */
#endif

    [28] = isr_usart2,             /* [28] USART 2 */
    [29] = isr_lpuart1,            /* [29] Low power UART 1 */

#if defined(CPU_MODEL_STM32L053R8) || defined(CPU_MODEL_STM32L073RZ) || \
    defined(CPU_MODEL_STM32L072CZ)
    [30] = isr_lcd,                /* [30] LCD */
    [31] = isr_usb                 /* [31] USB */
#endif
};
