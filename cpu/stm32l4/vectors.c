/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_stm32l4
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

/* STM32L4 specific interrupt vectors */
WEAK_DEFAULT void isr_wwdg(void);
WEAK_DEFAULT void isr_pvd_pvm(void);
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
WEAK_DEFAULT void isr_can1_tx(void);
WEAK_DEFAULT void isr_can1_rx0(void);
WEAK_DEFAULT void isr_can1_rx1(void);
WEAK_DEFAULT void isr_can1_sce(void);
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
WEAK_DEFAULT void isr_spi1(void);
WEAK_DEFAULT void isr_spi2(void);
WEAK_DEFAULT void isr_usart1(void);
WEAK_DEFAULT void isr_usart2(void);
WEAK_DEFAULT void isr_usart3(void);
WEAK_DEFAULT void isr_rtc_alarm(void);
WEAK_DEFAULT void isr_dfsdm1_flt3(void);
WEAK_DEFAULT void isr_tim8_brk(void);
WEAK_DEFAULT void isr_tim8_up(void);
WEAK_DEFAULT void isr_tim8_trg_com(void);
WEAK_DEFAULT void isr_tim8_cc(void);
WEAK_DEFAULT void isr_adc3(void);
WEAK_DEFAULT void isr_fmc(void);
WEAK_DEFAULT void isr_sdmmc1(void);
WEAK_DEFAULT void isr_tim5(void);
WEAK_DEFAULT void isr_spi3(void);
WEAK_DEFAULT void isr_uart4(void);
WEAK_DEFAULT void isr_uart5(void);
WEAK_DEFAULT void isr_tim6_dac(void);
WEAK_DEFAULT void isr_tim7(void);
WEAK_DEFAULT void isr_dma2_channel1(void);
WEAK_DEFAULT void isr_dma2_channel2(void);
WEAK_DEFAULT void isr_dma2_channel3(void);
WEAK_DEFAULT void isr_dma2_channel4(void);
WEAK_DEFAULT void isr_dma2_channel5(void);
WEAK_DEFAULT void isr_dfsdm1_flt0(void);
WEAK_DEFAULT void isr_dfsdm1_flt1(void);
WEAK_DEFAULT void isr_dfsdm1_flt2(void);
WEAK_DEFAULT void isr_comp(void);
WEAK_DEFAULT void isr_lptim1(void);
WEAK_DEFAULT void isr_lptim2(void);
WEAK_DEFAULT void isr_otg_fs(void);
WEAK_DEFAULT void isr_dma2_channel6(void);
WEAK_DEFAULT void isr_dma2_channel7(void);
WEAK_DEFAULT void isr_lpuart1(void);
WEAK_DEFAULT void isr_quadspi(void);
WEAK_DEFAULT void isr_i2c3_ev(void);
WEAK_DEFAULT void isr_i2c3_er(void);
WEAK_DEFAULT void isr_sai1(void);
WEAK_DEFAULT void isr_sai2(void);
WEAK_DEFAULT void isr_swpmi1(void);
WEAK_DEFAULT void isr_tsc(void);
WEAK_DEFAULT void isr_lcd(void);
WEAK_DEFAULT void isr_0(void);
WEAK_DEFAULT void isr_rng(void);
WEAK_DEFAULT void isr_fpu(void);
WEAK_DEFAULT void isr_crs(void);

/* CPU specific interrupt vector table */
ISR_VECTOR(1) const isr_t vector_cpu[CPU_IRQ_NUMOF] = {
    [ 0] = isr_wwdg,                /* [0] Window WatchDog Interrupt */
    [ 1] = isr_pvd_pvm,             /* [1] PVD/PVM1/PVM2/PVM3/PVM4 through EXTI Line detection Interrupts */
    [ 2] = isr_tamp_stamp,          /* [2] Tamper and TimeStamp interrupts through the EXTI line */
    [ 3] = isr_rtc_wkup,            /* [3] RTC Wakeup interrupt through the EXTI line */
    [ 4] = isr_flash,               /* [4] FLASH global Interrupt */
    [ 5] = isr_rcc,                 /* [5] RCC global Interrupt */
    [ 6] = isr_exti,                /* [6] EXTI Line0 Interrupt */
    [ 7] = isr_exti,                /* [7] EXTI Line1 Interrupt */
    [ 8] = isr_exti,                /* [8] EXTI Line2 Interrupt */
    [ 9] = isr_exti,                /* [9] EXTI Line3 Interrupt */
    [10] = isr_exti,                /* [10] EXTI Line4 Interrupt */
    [11] = isr_dma1_channel1,       /* [11] DMA1 Channel 1 global Interrupt */
    [12] = isr_dma1_channel2,       /* [12] DMA1 Channel 2 global Interrupt */
    [13] = isr_dma1_channel3,       /* [13] DMA1 Channel 3 global Interrupt */
    [14] = isr_dma1_channel4,       /* [14] DMA1 Channel 4 global Interrupt */
    [15] = isr_dma1_channel5,       /* [15] DMA1 Channel 5 global Interrupt */
    [16] = isr_dma1_channel6,       /* [16] DMA1 Channel 6 global Interrupt */
    [17] = isr_dma1_channel7,       /* [17] DMA1 Channel 7 global Interrupt */
    [18] = isr_adc1_2,              /* [18] ADC1 global Interrupt */
    [19] = isr_can1_tx,             /* [19] CAN1 TX Interrupt */
    [20] = isr_can1_rx0,            /* [20] CAN1 RX0 Interrupt */
    [21] = isr_can1_rx1,            /* [21] CAN1 RX1 Interrupt */
    [22] = isr_can1_sce,            /* [22] CAN1 SCE Interrupt */
    [23] = isr_exti,                /* [23] External Line[9:5] Interrupts */
    [24] = isr_tim1_brk_tim15,      /* [24] TIM1 Break interrupt and TIM15 global interrupt */
    [25] = isr_tim1_up_tim16,       /* [25] TIM1 Update Interrupt and TIM16 global interrupt */
    [26] = isr_tim1_trg_com_tim17,  /* [26] TIM1 Trigger and Commutation Interrupt */
    [27] = isr_tim1_cc,             /* [27] TIM1 Capture Compare Interrupt */
    [28] = isr_tim2,                /* [28] TIM2 global Interrupt */

    [31] = isr_i2c1_ev,             /* [31] I2C1 Event Interrupt */
    [32] = isr_i2c1_er,             /* [32] I2C1 Error Interrupt */

    [35] = isr_spi1,                /* [35] SPI1 global Interrupt */
    [37] = isr_usart1,              /* [37] USART1 global Interrupt */
    [38] = isr_usart2,              /* [38] USART2 global Interrupt */

    [40] = isr_exti,                /* [40] External Line[15:10] Interrupts */
    [41] = isr_rtc_alarm,           /* [41] RTC Alarm (A and B) through EXTI Line Interrupt */

    [51] = isr_spi3,                /* [51] SPI3 global Interrupt */

    [54] = isr_tim6_dac,            /* [54] TIM6 global and DAC1&2 underrun error interrupts */
    [55] = isr_tim7,                /* [55] TIM7 global interrupt */
    [56] = isr_dma2_channel1,       /* [56] DMA2 Channel 1 global Interrupt */
    [57] = isr_dma2_channel2,       /* [57] DMA2 Channel 2 global Interrupt */
    [58] = isr_dma2_channel3,       /* [58] DMA2 Channel 3 global Interrupt */
    [59] = isr_dma2_channel4,       /* [59] DMA2 Channel 4 global Interrupt */
    [60] = isr_dma2_channel5,       /* [60] DMA2 Channel 5 global Interrupt */

    [64] = isr_comp,                /* [64] COMP1 and COMP2 Interrupts */
    [65] = isr_lptim1,              /* [65] LP TIM1 interrupt */
    [66] = isr_lptim2,              /* [66] LP TIM2 interrupt */
    [67] = isr_otg_fs,              /* [67] USB OTG FS global Interrupt */
    [68] = isr_dma2_channel6,       /* [68] DMA2 Channel 6 global interrupt */
    [69] = isr_dma2_channel7,       /* [69] DMA2 Channel 7 global interrupt */
    [70] = isr_lpuart1,             /* [70] LP UART1 interrupt */
    [71] = isr_quadspi,             /* [71] Quad SPI global interrupt */
    [72] = isr_i2c3_ev,             /* [72] I2C3 event interrupt */
    [73] = isr_i2c3_er,             /* [73] I2C3 error interrupt */
    [74] = isr_sai1,                /* [74] Serial Audio Interface 1 global interrupt */

    [76] = isr_swpmi1,              /* [76] Serial Wire Interface 1 global interrupt */
    [77] = isr_tsc,                 /* [77] Touch Sense Controller global interrupt */

    [80] = isr_rng,                 /* [80] RNG global interrupt */
    [81] = isr_fpu,                 /* [81] FPU global interrupt */

#if defined(CPU_MODEL_STM32L476RG)
    [29] = isr_tim3,                /* [29] TIM3 global Interrupt */
    [30] = isr_tim4,                /* [30] TIM4 global Interrupt */
    [33] = isr_i2c2_ev,             /* [33] I2C2 Event Interrupt */
    [34] = isr_i2c2_er,             /* [34] I2C2 Error Interrupt */
    [36] = isr_spi2,                /* [36] SPI2 global Interrupt */
    [39] = isr_usart3,              /* [39] USART3 global Interrupt */
    [42] = isr_dfsdm1_flt3,         /* [42] DFSDM1 Filter 3 global Interrupt */
    [43] = isr_tim8_brk,            /* [43] TIM8 Break Interrupt */
    [44] = isr_tim8_up,             /* [44] TIM8 Update Interrupt */
    [45] = isr_tim8_trg_com,        /* [45] TIM8 Trigger and Commutation Interrupt */
    [46] = isr_tim8_cc,             /* [46] TIM8 Capture Compare Interrupt */
    [47] = isr_adc3,                /* [47] ADC3 global  Interrupt */
    [48] = isr_fmc,                 /* [48] FMC global Interrupt */
    [49] = isr_sdmmc1,              /* [49] SDMMC1 global Interrupt */
    [50] = isr_tim5,                /* [50] TIM5 global Interrupt */
    [52] = isr_uart4,               /* [52] UART4 global Interrupt */
    [53] = isr_uart5,               /* [53] UART5 global Interrupt */
    [61] = isr_dfsdm1_flt0,         /* [61] DFSDM1 Filter 0 global Interrupt */
    [62] = isr_dfsdm1_flt1,         /* [62] DFSDM1 Filter 1 global Interrupt */
    [63] = isr_dfsdm1_flt2,         /* [63] DFSDM1 Filter 2 global Interrupt */
    [75] = isr_sai2,                /* [75] Serial Audio Interface 2 global interrupt */
    [78] = isr_lcd,                 /* [78] LCD global interrupt */
#endif

#if defined(CPU_MODEL_STM32L432KC)
    [82] = isr_crs                  /* [82] CRS global interrupt */
#endif
};
