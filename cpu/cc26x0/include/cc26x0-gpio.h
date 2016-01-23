/*
 * Copyright (C) 2016 Leon George
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */
/**
 * @addtogroup cpu_cc26x0
 * @{
 *
 * @file
 * @brief           Driver for the cc26x0 GPIO controller
 *
 * @author          Leon George <leon@georgemail.eu>
 *
 * @defgroup cc26x0-gpio CC26x0 General-Purpose I/O
 * @{
 */

#ifndef CC26x0_GPIO_H
#define CC26x0_GPIO_H

#include "cc26x0.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup cpu_specific_peripheral_memory_map
 * @{
 */
#define GPIO_BASE                   (0x40022000)
/** @} */

typedef struct {
    reg32_t DOUT_FRAG[8];
    reg32_t __reserved1[24];
    reg32_t DOUT;
    reg32_t __reserved2[3];
    reg32_t DOUTSET;
    reg32_t __reserved3[3];
    reg32_t DOUTCLR;
    reg32_t __reserved4[3];
    reg32_t DOUTTGL;
    reg32_t __reserved5[3];
    reg32_t DIN;
    reg32_t __reserved6[3];
    reg32_t DOE;
    reg32_t __reserved7[3];
    reg32_t EVFLAGS;
} gpio_regs_t;

#define GPIO ((gpio_regs_t *) (GPIO_BASE))

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* CC26x0_GPIO_H */

/** @} */
