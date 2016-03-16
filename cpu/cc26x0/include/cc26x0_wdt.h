/*
 * Copyright (C) 2016 Leon George
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_cc26x0_definitions
 * @{
 *
 * @file
 * @brief           CC26x0 WDT register definitions
 */

#ifndef CC26x0_WDT_H
#define CC26x0_WDT_H

#include <cc26x0.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    reg32_t LOAD;
    reg32_t VALUE;
    reg32_t CTL;
    reg32_t ICR;
    reg32_t RIS;
    reg32_t MIS;
    reg32_t TEST;
    reg32_t INT_CAUS;
    reg32_t LOCK;
} wdt_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define WDT_BASE      0x40080000
/*@}*/

#define WDT           ((wdt_regs_t *) (WDT_BASE))

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* CC26x0_WDT_H */

/*@}*/
