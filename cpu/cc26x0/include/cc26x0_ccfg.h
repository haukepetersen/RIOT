/*
 * Copyright (C) 2016 Leon George
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */
/**
 * @ingroup         cpu_cc26x0_definitions
 * @{
 *
 * @file
 * @brief           CC26x0 CCFG register definitions
 */

#ifndef CC26x0_CCFG_H
#define CC26x0_CCFG_H

#include <cc26x0.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup cpu_specific_peripheral_memory_map
 * @{
 */
#define CCFG_BASE                           0x50003000 /**<  base address of CCFG memory */
/*@}*/

/* TODO not present in datasheet but explained in cc26x0ware-lds */
#define CCFG_SIZE_AND_DIS_FLAGS_DIS_GPRAM   0x00000004

typedef struct {
    reg32_t EXT_LF_CLK;
    reg32_t MODE_CONF_1;
    reg32_t SIZE_AND_DIS_FLAGS;
    reg32_t MODE_CONF;
    reg32_t VOLT_LOAD_0;
    reg32_t VOLT_LOAD_1;
    reg32_t RTC_OFFSET;
    reg32_t FREQ_OFFSET;
    reg32_t IEEE_MAC_0;
    reg32_t IEEE_MAC_1;
    reg32_t IEEE_BLE_0;
    reg32_t IEEE_BLE_1;
    reg32_t BL_CONFIG;
    reg32_t ERASE_CONF;
    reg32_t CCFG_TI_OPTIONS;
    reg32_t CCFG_TAP_DAP_0;
    reg32_t CCFG_TAP_DAP_1;
    reg32_t IMAGE_VALID_CONF;
    reg32_t CCFG_PROT_31_0;
    reg32_t CCFG_PROT_63_32;
    reg32_t CCFG_PROT_95_64;
    reg32_t CCFG_PROT_127_96;
} ccfg_regs_t;

#define CCFG ((ccfg_regs_t *) (CCFG_BASE + 0xFA8))

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* CC26x0_CCFG_H */

/*@}*/
