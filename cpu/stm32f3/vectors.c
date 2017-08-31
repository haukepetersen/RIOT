/*
 * Copyright (C) 2014-2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_stm32f3
 * @{
 *
 * @file
 * @brief       Interrupt vector definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
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

/* STM32F3 specific interrupt vectors */
WEAK_DEFAULT void isr_wwdg(void);
WEAK_DEFAULT void isr_pvd(void);
WEAK_DEFAULT void isr_tamp_stamp(void);
WEAK_DEFAULT void isr_rtc_wkup(void);
WEAK_DEFAULT void isr_flash(void);
WEAK_DEFAULT void isr_rcc(void);
WEAK_DEFAULT void isr_exti(void);
WEAK_DEFAULT void isr_dma1_channel1(void);
WEAK_DEFAULT void isr_dma1_channel2(void);
WEAK_DEFAULT void isr_dma1_channel3(void);
WEAK_DEFAULT void isr_dma1_channel4(void);
WEAK_DEFAULT void isr_dma1_channel5(void);
WEAK_DEFAULT void isr_dma1_channel6(void);
WEAK_DEFAULT void isr_dma1_channel7(void);
WEAK_DEFAULT void isr_adc1_2(void);
WEAK_DEFAULT void isr_usb_hp_can_tx(void);
WEAK_DEFAULT void isr_usb_lp_can_rx0(void);
WEAK_DEFAULT void isr_can_rx1(void);
WEAK_DEFAULT void isr_can_sce(void);
WEAK_DEFAULT void isr_tim1_brk_tim15(void);
WEAK_DEFAULT void isr_tim1_up_tim16(void);
WEAK_DEFAULT void isr_tim1_trg_com_tim17(void);
WEAK_DEFAULT void isr_tim1_cc(void);
WEAK_DEFAULT void isr_tim2(void);
WEAK_DEFAULT void isr_tim3(void);
WEAK_DEFAULT void isr_tim4(void);
WEAK_DEFAULT void isr_i2c1_ev(void);
WEAK_DEFAULT void isr_i2c1_er(void);
WEAK_DEFAULT void isr_i2c2_ev(void);
WEAK_DEFAULT void isr_i2c2_er(void);
WEAK_DEFAULT void isr_i2c3_ev(void);
WEAK_DEFAULT void isr_i2c3_er(void);
WEAK_DEFAULT void isr_spi1(void);
WEAK_DEFAULT void isr_spi2(void);
WEAK_DEFAULT void isr_usart1(void);
WEAK_DEFAULT void isr_usart2(void);
WEAK_DEFAULT void isr_usart3(void);
WEAK_DEFAULT void isr_rtc_alarm(void);
WEAK_DEFAULT void isr_usbwakeup(void);
WEAK_DEFAULT void isr_tim8_brk(void);
WEAK_DEFAULT void isr_tim8_up(void);
WEAK_DEFAULT void isr_tim8_trg_com(void);
WEAK_DEFAULT void isr_tim8_cc(void);
WEAK_DEFAULT void isr_adc3(void);
WEAK_DEFAULT void isr_fmc(void);
WEAK_DEFAULT void isr_spi3(void);
WEAK_DEFAULT void isr_uart4(void);
WEAK_DEFAULT void isr_uart5(void);
WEAK_DEFAULT void isr_tim6_dac(void);
WEAK_DEFAULT void isr_tim7(void);
WEAK_DEFAULT void isr_tim7_dac(void);
WEAK_DEFAULT void isr_dma2_channel1(void);
WEAK_DEFAULT void isr_dma2_channel2(void);
WEAK_DEFAULT void isr_dma2_channel3(void);
WEAK_DEFAULT void isr_dma2_channel4(void);
WEAK_DEFAULT void isr_dma2_channel5(void);
WEAK_DEFAULT void isr_adc4(void);
WEAK_DEFAULT void isr_comp1_2_3(void);
WEAK_DEFAULT void isr_comp4_5_6(void);
WEAK_DEFAULT void isr_comp7(void);
WEAK_DEFAULT void isr_hrtim(void);
WEAK_DEFAULT void isr_hrtim_tim_a(void);
WEAK_DEFAULT void isr_hrtim_tim_b(void);
WEAK_DEFAULT void isr_hrtim_tim_c(void);
WEAK_DEFAULT void isr_hrtim_tim_d(void);
WEAK_DEFAULT void isr_hrtim_tim_e(void);
WEAK_DEFAULT void isr_hrtim_flt(void);
WEAK_DEFAULT void isr_usb_hp(void);
WEAK_DEFAULT void isr_usb_lp(void);
WEAK_DEFAULT void isr_usbwakeup_rmp(void);
WEAK_DEFAULT void isr_tim20_brk(void);
WEAK_DEFAULT void isr_tim20_up(void);
WEAK_DEFAULT void isr_tim20_trg_com(void);
WEAK_DEFAULT void isr_tim20_cc(void);
WEAK_DEFAULT void isr_fpu(void);
WEAK_DEFAULT void isr_spi4(void);

/* CPU specific interrupt vector table */
ISR_VECTOR(1) const isr_t vector_cpu[CPU_IRQ_NUMOF] = {
    [0]  = isr_wwdg,               /* [0] Window WatchDog Interrupt */
    [1]  = isr_pvd,                /* [1] PVD through EXTI Line detection Interrupt */
    [2]  = isr_tamp_stamp,         /* [2] Tamper and TimeStamp interrupts through the EXTI line 19 */
    [3]  = isr_rtc_wkup,           /* [3] RTC Wakeup interrupt through the EXTI line 20 */
    [4]  = isr_flash,              /* [4] FLASH global Interrupt */
    [5]  = isr_rcc,                /* [5] RCC global Interrupt */
    [6]  = isr_exti,               /* [6] EXTI Line0 Interrupt */
    [7]  = isr_exti,               /* [7] EXTI Line1 Interrupt */
    [8]  = isr_exti,               /* [8] EXTI Line2 Interrupt and Touch Sense Controller Interrupt */
    [9]  = isr_exti,               /* [9] EXTI Line3 Interrupt */
    [10] = isr_exti,               /* [10] EXTI Line4 Interrupt */
    [11] = isr_dma1_channel1,      /* [11] DMA1 Channel 1 Interrupt */
    [12] = isr_dma1_channel2,      /* [12] DMA1 Channel 2 Interrupt */
    [13] = isr_dma1_channel3,      /* [13] DMA1 Channel 3 Interrupt */
    [14] = isr_dma1_channel4,      /* [14] DMA1 Channel 4 Interrupt */
    [15] = isr_dma1_channel5,      /* [15] DMA1 Channel 5 Interrupt */
    [16] = isr_dma1_channel6,      /* [16] DMA1 Channel 6 Interrupt */
    [17] = isr_dma1_channel7,      /* [17] DMA1 Channel 7 Interrupt */
    [18] = isr_adc1_2,             /* [18] ADC1 & ADC2 Interrupts */
    [19] = isr_usb_hp_can_tx,      /* [19] USB Device High Priority or CAN TX Interrupts */
    [20] = isr_usb_lp_can_rx0,     /* [20] USB Device Low Priority or CAN RX0 Interrupts */
    [21] = isr_can_rx1,            /* [21] CAN RX1 Interrupt */
    [22] = isr_can_sce,            /* [22] CAN SCE Interrupt */
    [23] = isr_exti,               /* [23] External Line[9:5] Interrupts */
    [24] = isr_tim1_brk_tim15,     /* [24] TIM1 Break and TIM15 Interrupts */
    [25] = isr_tim1_up_tim16,      /* [25] TIM1 Update and TIM16 Interrupts */
    [26] = isr_tim1_trg_com_tim17, /* [26] TIM1 Trigger and Commutation and TIM17 Interrupt */
    [27] = isr_tim1_cc,            /* [27] TIM1 Capture Compare Interrupt */
    [28] = isr_tim2,               /* [28] TIM2 global Interrupt */

#if defined(CPU_MODEL_STM32F303K8) || defined(CPU_MODEL_STM32F334R8) || \
    defined(CPU_MODEL_STM32F303VC) || defined(CPU_MODEL_STM32F303VC) || \
    defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [29] = isr_tim3,               /* [29] TIM3 global Interrupt */
#endif

#if defined(CPU_MODEL_STM32F334R8) || defined(CPU_MODEL_STM32F303VC) || \
    defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [30] = isr_tim4,               /* [30] TIM4 global Interrupt */
#endif

    [31] = isr_i2c1_ev,            /* [31] I2C1 Event Interrupt & EXTI Line23 Interrupt (I2C1 wakeup) */
    [32] = isr_i2c1_er,            /* [32] I2C1 Error Interrupt */

#if defined(CPU_MODEL_STM32F302R8) || defined(CPU_MODEL_STM32F303VC) || \
    defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [33] = isr_i2c2_ev,            /* [33] I2C2 Event Interrupt & EXTI Line24 Interrupt (I2C2 wakeup) */
    [34] = isr_i2c2_er,            /* [34] I2C2 Error Interrupt */
#endif

#if defined(CPU_MODEL_STM32F303K8) || defined(CPU_MODEL_STM32F334R8) || \
    defined(CPU_MODEL_STM32F303VC) || defined(CPU_MODEL_STM32F303RE) || \
    defined(CPU_MODEL_STM32F303ZE)
    [35] = isr_spi1,               /* [35] SPI1 global Interrupt */
#endif

#if defined(CPU_MODEL_STM32F302R8) || defined(CPU_MODEL_STM32F303VC) || \
    defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [36] = isr_spi2,               /* [36] SPI2 global Interrupt */
#endif

    [37] = isr_usart1,             /* [37] USART1 global Interrupt & EXTI Line25 Interrupt (USART1 wakeup) */
    [38] = isr_usart2,             /* [38] USART2 global Interrupt & EXTI Line26 Interrupt (USART2 wakeup) */
    [39] = isr_usart3,             /* [39] USART3 global Interrupt & EXTI Line28 Interrupt (USART3 wakeup) */
    [40] = isr_exti,               /* [40] External Line[15:10] Interrupts */
    [41] = isr_rtc_alarm,          /* [41] RTC Alarm (A and B) through EXTI Line 17 Interrupt */

#if defined(CPU_MODEL_STM32F302R8) || defined(CPU_MODEL_STM32F303VC) || \
    defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [42] = isr_usbwakeup,          /* [42] USB Wakeup Interrupt */
#endif

#if defined(CPU_MODEL_STM32F303VC) || defined(CPU_MODEL_STM32F303RE) || \
    defined(CPU_MODEL_STM32F303ZE)
    [43] = isr_tim8_brk,           /* [43] TIM8 Break Interrupt */
    [44] = isr_tim8_up,            /* [44] TIM8 Update Interrupt */
    [45] = isr_tim8_trg_com,       /* [45] TIM8 Trigger and Commutation Interrupt */
    [46] = isr_tim8_cc,            /* [46] TIM8 Capture Compare Interrupt */
    [47] = isr_adc3,               /* [47] ADC3 global Interrupt */
#endif

#if defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [48] = isr_fmc,                /* [48] FMC global Interrupt */
#endif

#if defined(CPU_MODEL_STM32F302R8) || defined(CPU_MODEL_STM32F303VC) || \
    defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [51] = isr_spi3,               /* [51] SPI3 global Interrupt */
#endif

#if defined(CPU_MODEL_STM32F303VC) || defined(CPU_MODEL_STM32F303RE) || \
    defined(CPU_MODEL_STM32F303ZE)
    [52] = isr_uart4,              /* [52] UART4 global Interrupt & EXTI Line34 Interrupt (UART4 wakeup) */
    [53] = isr_uart5,              /* [53] UART5 global Interrupt & EXTI Line35 Interrupt (UART5 wakeup) */
#endif

    [54] = isr_tim6_dac,           /* [54] TIM6 global and DAC channel 1&2 underrun error  interrupts */

#if defined(CPU_MODEL_STM32F303K8) || defined(CPU_MODEL_STM32F303VC) || \
    defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [55] = isr_tim7_dac,           /* [55] TIM7 global and DAC2 channel1 underrun error Interrupt */
#endif

#if defined(CPU_MODEL_STM32F303VC) || defined(CPU_MODEL_STM32F303RE) || \
    defined(CPU_MODEL_STM32F303ZE)
    [56] = isr_dma2_channel1,      /* [56] DMA2 Channel 1 global Interrupt */
    [57] = isr_dma2_channel2,      /* [57] DMA2 Channel 2 global Interrupt */
    [58] = isr_dma2_channel3,      /* [58] DMA2 Channel 3 global Interrupt */
    [59] = isr_dma2_channel4,      /* [59] DMA2 Channel 4 global Interrupt */
    [60] = isr_dma2_channel5,      /* [60] DMA2 Channel 5 global Interrupt */
    [61] = isr_adc4,               /* [61] ADC4  global Interrupt */
#endif

    [64] = isr_comp1_2_3,          /* [64] COMP1, COMP2 and COMP3 global Interrupt via EXTI Line21, 22 and 29 */
    [65] = isr_comp4_5_6,          /* [65] COMP4, COMP5 and COMP6 global Interrupt via EXTI Line30, 31 and 32 */

#if defined(CPU_MODEL_STM32F303VC) || defined(CPU_MODEL_STM32F303RE) || \
    defined(CPU_MODEL_STM32F303ZE)
    [66] = isr_comp7,              /* [66] COMP7 global Interrupt via EXTI Line33 */
#endif

#if defined(CPU_MODEL_STM32F334R8)
    [67] = isr_hrtim,              /* [67] HRTIM Master Timer global Interrupts */
    [68] = isr_hrtim_tim_a,        /* [68] HRTIM Timer A global Interrupt */
    [69] = isr_hrtim_tim_b,        /* [69] HRTIM Timer B global Interrupt */
    [70] = isr_hrtim_tim_c,        /* [70] HRTIM Timer C global Interrupt */
    [71] = isr_hrtim_tim_d,        /* [71] HRTIM Timer D global Interrupt */
    [72] = isr_hrtim_tim_e,        /* [72] HRTIM Timer E global Interrupt */
    [73] = isr_hrtim_flt,          /* [73] HRTIM Fault global Interrupt */
#endif

#if defined(CPU_MODEL_STM32F302R8) || defined(CPU_MODEL_STM32F303RE) || \
    defined(CPU_MODEL_STM32F303ZE)
    [72] = isr_i2c3_ev,            /* [72] I2C3 event interrupt */
    [73] = isr_i2c3_er,            /* [73] I2C3 error interrupt */
#endif

#if defined(CPU_MODEL_STM32F302R8) || defined(CPU_MODEL_STM32F303VC) || \
    defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [74] = isr_usb_hp,             /* [74] USB High Priority global Interrupt remap */
    [75] = isr_usb_lp,             /* [75] USB Low Priority global Interrupt  remap */
    [76] = isr_usbwakeup_rmp,      /* [76] USB Wakeup Interrupt remap */
#endif

#if defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [77] = isr_tim20_brk,          /* [77] TIM20 Break Interrupt */
    [78] = isr_tim20_up,           /* [78] TIM20 Update Interrupt */
    [79] = isr_tim20_trg_com,      /* [79] TIM20 Trigger and Commutation Interrupt */
    [80] = isr_tim8_cc,            /* [80] TIM20 Capture Compare Interrupt */
#endif

    [81] = isr_fpu,                /* [81] Floating point Interrupt */

#if defined(CPU_MODEL_STM32F303RE) || defined(CPU_MODEL_STM32F303ZE)
    [84] = isr_spi4,               /* [84] SPI4 global Interrupt */
#endif
};
