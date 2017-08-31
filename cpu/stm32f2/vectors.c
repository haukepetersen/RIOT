/*
 * Copyright (C) 2015 Engineering-Spirit
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_stm32f2
 * @{
 *
 * @file
 * @brief       Interrupt vector definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Nick v. IJzendoorn <nijzendoorn@engineering-spirit.nl>
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

/* Cortex-M common interrupt vectors */
WEAK_DEFAULT void isr_svc(void);
WEAK_DEFAULT void isr_pendsv(void);
WEAK_DEFAULT void isr_systick(void);
/* STM32F2 specific interrupt vectors */
WEAK_DEFAULT void isr_wwdg(void);
WEAK_DEFAULT void isr_pvd(void);
WEAK_DEFAULT void isr_tamp_stamp(void);
WEAK_DEFAULT void isr_rtc_wkup(void);
WEAK_DEFAULT void isr_flash(void);
WEAK_DEFAULT void isr_rcc(void);
WEAK_DEFAULT void isr_exti(void);
WEAK_DEFAULT void isr_dma1_stream0(void);
WEAK_DEFAULT void isr_dma1_stream1(void);
WEAK_DEFAULT void isr_dma1_stream2(void);
WEAK_DEFAULT void isr_dma1_stream3(void);
WEAK_DEFAULT void isr_dma1_stream4(void);
WEAK_DEFAULT void isr_dma1_stream5(void);
WEAK_DEFAULT void isr_dma1_stream6(void);
WEAK_DEFAULT void isr_adc(void);
WEAK_DEFAULT void isr_can1_tx(void);
WEAK_DEFAULT void isr_can1_rx0(void);
WEAK_DEFAULT void isr_can1_rx1(void);
WEAK_DEFAULT void isr_can1_sce(void);
WEAK_DEFAULT void isr_tim1_brk_tim9(void);
WEAK_DEFAULT void isr_tim1_up_tim10(void);
WEAK_DEFAULT void isr_tim1_trg_com_tim11(void);
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
WEAK_DEFAULT void isr_otg_fs_wkup(void);
WEAK_DEFAULT void isr_tim8_brk_tim12(void);
WEAK_DEFAULT void isr_tim8_up_tim13(void);
WEAK_DEFAULT void isr_tim8_trg_com_tim14(void);
WEAK_DEFAULT void isr_tim8_cc(void);
WEAK_DEFAULT void isr_dma1_stream7(void);
WEAK_DEFAULT void isr_fsmc(void);
WEAK_DEFAULT void isr_sdio(void);
WEAK_DEFAULT void isr_tim5(void);
WEAK_DEFAULT void isr_spi3(void);
WEAK_DEFAULT void isr_uart4(void);
WEAK_DEFAULT void isr_uart5(void);
WEAK_DEFAULT void isr_tim6_dac(void);
WEAK_DEFAULT void isr_tim7(void);
WEAK_DEFAULT void isr_dma2_stream0(void);
WEAK_DEFAULT void isr_dma2_stream1(void);
WEAK_DEFAULT void isr_dma2_stream2(void);
WEAK_DEFAULT void isr_dma2_stream3(void);
WEAK_DEFAULT void isr_dma2_stream4(void);
WEAK_DEFAULT void isr_eth(void);
WEAK_DEFAULT void isr_eth_wkup(void);
WEAK_DEFAULT void isr_can2_tx(void);
WEAK_DEFAULT void isr_can2_rx0(void);
WEAK_DEFAULT void isr_can2_rx1(void);
WEAK_DEFAULT void isr_can2_sce(void);
WEAK_DEFAULT void isr_otg_fs(void);
WEAK_DEFAULT void isr_dma2_stream5(void);
WEAK_DEFAULT void isr_dma2_stream6(void);
WEAK_DEFAULT void isr_dma2_stream7(void);
WEAK_DEFAULT void isr_usart6(void);
WEAK_DEFAULT void isr_i2c3_ev(void);
WEAK_DEFAULT void isr_i2c3_er(void);
WEAK_DEFAULT void isr_otg_hs_ep1_out(void);
WEAK_DEFAULT void isr_otg_hs_ep1_in(void);
WEAK_DEFAULT void isr_otg_hs_wkup(void);
WEAK_DEFAULT void isr_otg_hs(void);
WEAK_DEFAULT void isr_dcmi(void);
WEAK_DEFAULT void isr_cryp(void);
WEAK_DEFAULT void isr_hash_rng(void);

/* CPU specific interrupt vector table */
ISR_VECTOR(1) const isr_t vector_cpu[CPU_IRQ_NUMOF] = {
    [0]  = isr_wwdg,               /* [0] Window WatchDog */
    [1]  = isr_pvd,                /* [1] PVD through EXTI Line detection */
    [2]  = isr_tamp_stamp,         /* [2] Tamper and TimeStamps through the EXTI line */
    [3]  = isr_rtc_wkup,           /* [3] RTC Wakeup through the EXTI line */
    [4]  = isr_flash,              /* [4] FLASH */
    [5]  = isr_rcc,                /* [5] RCC */
    [6]  = isr_exti,               /* [6] EXTI Line0 */
    [7]  = isr_exti,               /* [7] EXTI Line1 */
    [8]  = isr_exti,               /* [8] EXTI Line2 */
    [9]  = isr_exti,               /* [9] EXTI Line3 */
    [10] = isr_exti,               /* [10] EXTI Line4 */
    [11] = isr_dma1_stream0,       /* [11] DMA1 Stream 0 */
    [12] = isr_dma1_stream1,       /* [12] DMA1 Stream 1 */
    [13] = isr_dma1_stream2,       /* [13] DMA1 Stream 2 */
    [14] = isr_dma1_stream3,       /* [14] DMA1 Stream 3 */
    [15] = isr_dma1_stream4,       /* [15] DMA1 Stream 4 */
    [16] = isr_dma1_stream5,       /* [16] DMA1 Stream 5 */
    [17] = isr_dma1_stream6,       /* [17] DMA1 Stream 6 */
    [18] = isr_adc,                /* [18] ADC1, ADC2 and ADC3s */
    [19] = isr_can1_tx,            /* [19] CAN1 TX */
    [20] = isr_can1_rx0,           /* [20] CAN1 RX0 */
    [21] = isr_can1_rx1,           /* [21] CAN1 RX1 */
    [22] = isr_can1_sce,           /* [22] CAN1 SCE */
    [23] = isr_exti,               /* [23] External Line[9:5]s */
    [24] = isr_tim1_brk_tim9,      /* [24] TIM1 Break and TIM9 */
    [25] = isr_tim1_up_tim10,      /* [25] TIM1 Update and TIM10 */
    [26] = isr_tim1_trg_com_tim11, /* [26] TIM1 Trigger and Commutation and TIM11 */
    [27] = isr_tim1_cc,            /* [27] TIM1 Capture Compare */
    [28] = isr_tim2,               /* [28] TIM2 */
    [29] = isr_tim3,               /* [29] TIM3 */
    [30] = isr_tim4,               /* [30] TIM4 */
    [31] = isr_i2c1_ev,            /* [31] I2C1 Event */
    [32] = isr_i2c1_er,            /* [32] I2C1 Error */
    [33] = isr_i2c2_ev,            /* [33] I2C2 Event */
    [34] = isr_i2c2_er,            /* [34] I2C2 Error */
    [35] = isr_spi1,               /* [35] SPI1 */
    [36] = isr_spi2,               /* [36] SPI2 */
    [37] = isr_usart1,             /* [37] USART1 */
    [38] = isr_usart2,             /* [38] USART2 */
    [39] = isr_usart3,             /* [39] USART3 */
    [40] = isr_exti,               /* [40] External Line[15:10]s */
    [41] = isr_rtc_alarm,          /* [41] RTC Alarm (A and B) through EXTI Line */
    [42] = isr_otg_fs_wkup,        /* [42] USB OTG FS Wakeup through EXTI line */
    [43] = isr_tim8_brk_tim12,     /* [43] TIM8 Break and TIM12 */
    [44] = isr_tim8_up_tim13,      /* [44] TIM8 Update and TIM13 */
    [45] = isr_tim8_trg_com_tim14, /* [45] TIM8 Trigger and Commutation and TIM14 */
    [46] = isr_tim8_cc,            /* [46] TIM8 Capture Compare */
    [47] = isr_dma1_stream7,       /* [47] DMA1 Stream7 */
    [48] = isr_fsmc,               /* [48] FSMC */
    [49] = isr_sdio,               /* [49] SDIO */
    [50] = isr_tim5,               /* [50] TIM5 */
    [51] = isr_spi3,               /* [51] SPI3 */
    [52] = isr_uart4,              /* [52] UART4 */
    [53] = isr_uart5,              /* [53] UART5 */
    [54] = isr_tim6_dac,           /* [54] TIM6 and DAC1&2 underrun errors */
    [55] = isr_tim7,               /* [55] TIM7 */
    [56] = isr_dma2_stream0,       /* [56] DMA2 Stream 0 */
    [57] = isr_dma2_stream1,       /* [57] DMA2 Stream 1 */
    [58] = isr_dma2_stream2,       /* [58] DMA2 Stream 2 */
    [59] = isr_dma2_stream3,       /* [59] DMA2 Stream 3 */
    [60] = isr_dma2_stream4,       /* [60] DMA2 Stream 4 */

#if defined(CPU_MODEL_STM32F207ZG) || defined(CPU_MODEL_STM32F217ZG)
    [61] = isr_eth,                /* [61] Ethernet */
    [62] = isr_eth_wkup,           /* [62] Ethernet Wakeup through EXTI line */
#endif

    [63] = isr_can2_tx,            /* [63] CAN2 TX */
    [64] = isr_can2_rx0,           /* [64] CAN2 RX0 */
    [65] = isr_can2_rx1,           /* [65] CAN2 RX1 */
    [66] = isr_can2_sce,           /* [66] CAN2 SCE */
    [67] = isr_otg_fs,             /* [67] USB OTG FS */
    [68] = isr_dma2_stream5,       /* [68] DMA2 Stream 5 */
    [69] = isr_dma2_stream6,       /* [69] DMA2 Stream 6 */
    [70] = isr_dma2_stream7,       /* [70] DMA2 Stream 7 */
    [71] = isr_usart6,             /* [71] USART6 */
    [72] = isr_i2c3_ev,            /* [72] I2C3 event */
    [73] = isr_i2c3_er,            /* [73] I2C3 error */
    [74] = isr_otg_hs_ep1_out,     /* [74] USB OTG HS End Point 1 Out */
    [75] = isr_otg_hs_ep1_in,      /* [75] USB OTG HS End Point 1 In */
    [76] = isr_otg_hs_wkup,        /* [76] USB OTG HS Wakeup through EXTI */
    [77] = isr_otg_hs,             /* [77] USB OTG HS */

#if defined(CPU_MODEL_STM32F207ZG) || defined(CPU_MODEL_STM32F217ZG)
    [78] = isr_dcmi,               /* [78] DCMI */
#endif

#if defined(CPU_MODEL_STM32F215RG) || defined (CPU_MODEL_STM32F215VG) || \
    defined(CPU_MODEL_STM32F217ZG)
    [79] = isr_cryp,               /* [79] CRYP crypto */
#endif

    [80] = isr_hash_rng            /* [80] Hash and Rng */
};
