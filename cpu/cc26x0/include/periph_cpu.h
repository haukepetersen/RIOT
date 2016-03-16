/*
 * Copyright (C) 2016 Leon George
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_cc26x0
 * @{
 *
 * @file
 * @brief           CPU specific definitions for internal peripheral handling
 *
 * @author          Leon M. George <leon@georgemail.de>
 */

#ifndef PERIPH_CPU_H_
#define PERIPH_CPU_H_

#include "cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HAVE_GPIO_FLANK_T
typedef enum {
    GPIO_FALLING = IOCFG_EDGEDET_FALLING,
    GPIO_RISING = IOCFG_EDGEDET_RISING,
    GPIO_BOTH = IOCFG_EDGEDET_BOTH
} gpio_flank_t;


#define HAVE_GPIO_PP_T
typedef enum {
    GPIO_NOPULL = 0,
    GPIO_PULLDOWN = IOCFG_PULLCTL_DOWN,
    GPIO_PULLUP = IOCFG_PULLCTL_UP,
} gpio_pp_t;

/**
 * @brief   Timer configuration options
 */
typedef struct {
    gpt_reg_t *dev;
    uint8_t num;
    uint8_t irqn;
} timer_conf_t;

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_H_ */
/** @} */
