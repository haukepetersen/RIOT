/*
 * Copyright (C) 2016 Leon George
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_cc26x0_i2c_definitions
 * @{
 *
 * @file
 * @brief           CC26x0 MCU I/O register definitions
 *
 * @author          Leon George <leon@georgemail.eu>
 */

#ifndef CC26x0_I2C_H
#define CC26x0_I2C_H

#include "cc26x0.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    reg32_t SOAR;
    union {
        reg32_t SSTAT;
        reg32_t SCTL;
    };
    reg32_t SDR;
    reg32_t SIMR;
    reg32_t SRIS;
    reg32_t SMIS;
    reg32_t SICR;
    reg32_t __reserved[0x1F9];
    reg32_t MSA;
    union {
        reg32_t MSTAT;
        reg32_t MCTRL;
    };
    reg32_t MDR;
    reg32_t MTPR;
    reg32_t MIMR;
    reg32_t MRIS;
    reg32_t MMIS;
    reg32_t MICR;
    reg32_t MCR;
} i2c_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define I2C_BASE           (PERIPH_BASE + 0x2000)
/** @} */

#define I2C ((i2c_regs_t *) (I2C_BASE))

#ifdef __cplusplus
}
#endif

#endif /* CC26x0_I2C_H */

/*@}*/
