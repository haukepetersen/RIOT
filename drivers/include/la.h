/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    driver_lh Logic Analyzer Hooks
 * @ingroup     drivers
 * @brief       Some pin hook for triggering a logic analyzer
 *
 * @{
 * @file
 * @brief       Low-level GPIO peripheral driver interface definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef CPU_LA_H
#define CPU_LA_H

#include "cpu.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LA0_H          (GPIOD->BSRRL = (1 << 0))
#define LA0_L          (GPIOD->BSRRH = (1 << 0))
#define LA0_T          (GPIOD->ODR ^= (1 << 0))
#define LA1_H          (GPIOD->BSRRL = (1 << 1))
#define LA1_L          (GPIOD->BSRRH = (1 << 1))
#define LA1_T          (GPIOD->ODR ^= (1 << 1))
#define LA2_H          (GPIOD->BSRRL = (1 << 2))
#define LA2_L          (GPIOD->BSRRH = (1 << 2))
#define LA2_T          (GPIOD->ODR ^= (1 << 2))
#define LA3_H          (GPIOD->BSRRL = (1 << 3))
#define LA3_L          (GPIOD->BSRRH = (1 << 3))
#define LA3_T          (GPIOD->ODR ^= (1 << 3))
#define LA4_H          (GPIOE->BSRRL = (1 << 0))
#define LA4_L          (GPIOE->BSRRH = (1 << 0))
#define LA4_T          (GPIOE->ODR ^= (1 << 0))
#define LA5_H          (GPIOE->BSRRL = (1 << 1))
#define LA5_L          (GPIOE->BSRRH = (1 << 1))
#define LA5_T          (GPIOE->ODR ^= (1 << 1))
#define LA6_H          (GPIOE->BSRRL = (1 << 2))
#define LA6_L          (GPIOE->BSRRH = (1 << 2))
#define LA6_T          (GPIOE->ODR ^= (1 << 2))
#define LA7_H          (GPIOE->BSRRL = (1 << 3))
#define LA7_L          (GPIOE->BSRRH = (1 << 3))
#define LA7_T          (GPIOE->ODR ^= (1 << 3))

#ifdef __cplusplus
}
#endif

#endif /* CPU_LA_H */
/** @} */
