/*
 * Copyright (C) 2016 Leon George
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */
/**
 * @ingroup     cpu_cc26x0
 * @{
 *
 * @file
 * @brief       CCFG-definition
 *
 */

/* TODO this code is not used. someone would have to add a section to the linker script */

#ifdef OUTPUT_CCFG

#ifndef CCFG_BACKDOOR_ENABLE
#warning "BOOTLOADER_BACKDOOR_ENABLE is not set"
#define __BACKDOOR_CONFIG (0)
#else
#define __BACKDOOR_ENABLE       0xC5
#define __BACKDOOR_PIN_UMBER    BOOTLOADER_BACKDOOR_PIN
#define __BACKDOOR_ACTIVE_LEVEL BOOTLOADER_BACKDOOR_LEVEL
#define __BACKDOOR_CONFIG (__BACKDOOR_ENABLE | (__BACKDOOR_PIN_UMBER << 8) | (__BACKDOOR_ACTIVE_LEVEL << 16))
#endif

#define CCFG_BL_CONFIG (__BACKDOOR_CONFIG | 0xC5000000)

/**
 * @brief customer configuration (CCFG)
 */
__attribute__((section(".ccfg"), used))
const uint32_t ccfg[] = {
    0x01800000,         /* O_EXT_LF_CLK */
    0x0082FF10,         /* MODE_CONF_1 */
    0x0058FFF9,         /* SIZE_AND_DIS_FLAGS: XOSC_OVR disabled, ALT_DCDC enabled, GPRAM disabled ( | 4 to enable) */
    0xF39FFF3A,         /* MODE_CONF: XOSC_LF (32.768kHz), XOSC_NO_CAP_MOD, XOSC_FREQ_24MHz_xtal, DCDC active/recharge */
    0xFFFFFFFF,         /* VOLT_LOAD_0 */
    0xFFFFFFFF,         /* VOLT_LOAD_1 */
    0xFFFFFFFF,         /* RTC_OFFSET */
    0xFFFFFFFF,         /* FREQ_OFFSET */
    0xFFFFFFFF,         /* IEEE_MAC_0 */
    0xFFFFFFFF,         /* IEEE_MAC_1 */
    0xFFFFFFFF,         /* IEEE_BLE_0 */
    0xFFFFFFFF,         /* IEEE_BLE_1 */
    CCFG_BL_CONFIG,     /* BL_CONFIG */
    0x00000101,         /* ERASE_CONF */
    0x00000000,         /* CCFG_TI_OPTIONS */
    0x00C5C5C5,         /* CCFG_TAP_DAP_0 */
    0x00C5C5C5,         /* CCFG_TAP_DAP_1 */
    0x00000000,         /* IMAGE_VALID_CONF */
    0xFFFFFFFF,         /* CCFG_PROT_31_0 */
    0xFFFFFFFF,         /* CCFG_PROT_63_32 */
    0xFFFFFFFF,         /* CCFG_PROT_95_64 */
    0xFFFFFFFF          /* CCFG_PROT_127_96 */
};
#endif

/** @} */
