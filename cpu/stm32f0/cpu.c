/*
 * Copyright (C) 2014-2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_stm32f0
 * @{
 *
 * @file
 * @brief       Implementation of the CPU initialization
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "cpu.h"
#include "periph_conf.h"
#include "periph/init.h"

/* get base clock configuration */
#if CLOCK_HSE       /* use external crystal with given clock rate */
#define CLK_EN                  RCC_CR_HSEON
#define CLK_STAT                RCC_CR_HSERDY
#define CLK_SRC                 RCC_CFGR_SW_HSE
#define CKL_SRC_STAT            RCC_CFGR_SWS_HSE
#define PLL_IN                  (CLOCK_HSE / 2)
#define PLL_SRC                 (RCC_CFGR_PLLSRC_HSE_PREDIV)
#else               /* use internal 8MHz HSI oscillator */
#define CLK_EN                  RCC_CR_HSION
#define CLK_STAT                RCC_CR_HSIRDY
#define CLK_SRC                 RCC_CFGR_SW_HSI
#define CLK_SRC_STAT            RCC_CFGR_SWS_HSI
#define PLL_IN                  (8000000 / 2)
#define PLL_SRC                 (RCC_CFGR_PLLSRC_HSI_PREDIV)
#else
#error "Please provide CLOCK_HSE in your board's periph_conf.h"
#endif

/* further PLL settings */
#define PLL_MUL_VAL             (CLOCK_CORECLOCK / PLL_IN)
#if (PLL_MUL_VAL > 16 || PLL_MUL_VAL < 2)
#error "PLL configuration: multiplier value is out of range"
#endif
#define PLL_MUL                 ((PLL_MUL_VAL - 2) << 18)

/* set default bus prescalers */
#ifndef CLOCK_AHB_DIV
#define CLOCK_AHB_DIV               RCC_CFGR_HPRE_DIV1
#endif
#ifndef CLOCK_APB_DIV
#define CLOCK_APB_DIV               RCC_CFGR_PPRE_DIV1
#endif

/* figure out flash wait states that we need */
#if (CLOCK_CORECLOCK <= 24000000)
#define FLASH_WS                    (0)
#else
#define FLASH_WS                    (FLASH_ACR_LATENCY)
#endif

/**
 * @brief   Configure the controllers clock system
 *
 * We use the following default configuration:
 * - if configured, use HSE with given clock rate (CLOCK_HSE in periph_conf.h)
 * - if HSE is not available (CLOCK_HSE := 0), use 8MHz HSI as base clock
 * - we use the PLL to achieve desired core clock
 */
static void clock_init(void)
{
    /* disable interrupts (global + RCC) */
    unsigned is = irq_disable();
    RCC->CIR = 0;

    /* set flash wait states to maximum during configuration */
    FLASH->ACR = FLASH_ACR_LATENCY;

    /* enable the base clock */
    RCC->CR |= (CLK_EN);
    while (!(RCC->CR & CLK_STAT)) {}
    /* use base clock to drive the system during further initialization and
     * set desired peripheral bus prescalers */
    RCC_CFGR = (CLOCK_AHB_DIV | CLOCK_APB_DIV | CLK_SRC);
    while ((RCC_CFGR & RCC_CFGR_SWS) != CKL_SRC_STAT) {}
    /* show down all clocks except the base clock -> now we are in a defined
     * state */
    RCC->CR = CLK_EN;

    /* configure the PLL: we simply re-write the previous CFGR settings once
     * more + we add the new PWM configuration */
    RCC->CFGR2 = 1;         /* pre-div fixed to 2 */
    RCC->CFGR = (CLOCK_AHB_DIV | CLOCK_APB_DIV | CLK_SRC | PLL_MUL | PLL_SRC);

    /* enable PLL */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {}

    /* configure PLL as system clock source */
    RCC->CFGR &= ~(RCC_CFGR_SW);
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}

    /* now we just need to enable the correct low speed clock */
#if CLOCK_LSE

#else

#endif

    /* and set the flash configuration to the actual used values */
    FLASH->ACR = (FLASH_WS | FLASH_ACR_PRFTBE);

    irq_restore(is);
}

/**
 * @brief   Initialize the CPU, set IRQ priorities
 */
void cpu_init(void)
{
    /* initialize the Cortex-M core */
    cortexm_init();
    /* initialize the clock system */
    clock_init();
    /* trigger static peripheral initialization */
    periph_init();
}
