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
 * @brief           CC26x0 FCFG register definitions
 */

#ifndef CC26x0_FCFG_H
#define CC26x0_FCFG_H

#include <cc26x0.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup cpu_specific_peripheral_memory_map
 * @{
 */
#define FCFG_BASE                           0x50001000 /**<  base address of FCFG memory */
/*@}*/

typedef struct {
    uint8_t __reserved1[0xA0];
    /* TODO does it pad here? */
    reg32_t MISC_CONF_1;
    reg32_t __reserved2[8];
    reg32_t CONFIG_RF_FRONTEND_DIV5;
    reg32_t CONFIG_RF_FRONTEND_DIV6;
    reg32_t CONFIG_RF_FRONTEND_DIV10;
    reg32_t CONFIG_RF_FRONTEND_DIV12;
    reg32_t CONFIG_RF_FRONTEND_DIV15;
    reg32_t CONFIG_RF_FRONTEND_DIV30;
    reg32_t CONFIG_SYNTH_DIV5;
    reg32_t CONFIG_SYNTH_DIV6;
    reg32_t CONFIG_SYNTH_DIV10;
    reg32_t CONFIG_SYNTH_DIV12;
    reg32_t CONFIG_SYNTH_DIV15;
    reg32_t CONFIG_SYNTH_DIV30;
    reg32_t CONFIG_MISC_ADC_DIV5;
    reg32_t CONFIG_MISC_ADC_DIV6;
    reg32_t CONFIG_MISC_ADC_DIV10;
    reg32_t CONFIG_MISC_ADC_DIV12;
    reg32_t CONFIG_MISC_ADC_DIV15;
    reg32_t CONFIG_MISC_ADC_DIV30;
    reg32_t __reserved3[3];
    reg32_t SHDW_DIE_ID_0;
    reg32_t SHDW_DIE_ID_1;
    reg32_t SHDW_DIE_ID_2;
    reg32_t SHDW_DIE_ID_3;
    reg32_t __reserved4[4];
    reg32_t SHDW_OSC_BIAS_LDO_TRIM;
    reg32_t SHDW_ANA_TRIM;
    reg32_t __reserved5[9];
    reg32_t FLASH_NUMBER;
    reg32_t FLASH_COORDINATE;
    reg32_t FLASH_E_P;
    reg32_t FLASH_C_E_P_R;
    reg32_t FLASH_P_R_PV;
    reg32_t FLASH_EH_SEQ;
    reg32_t FLASH_VHV_E;
    reg32_t FLASH_PP;
    reg32_t FLASH_PROG_EP;
    reg32_t FLASH_ERA_PW;
    reg32_t FLASH_VHV;
    reg32_t FLASH_VHV_PV;
    reg32_t FLASH_V;
    reg32_t __reserved6[0x3E];
    reg32_t USER_ID;
    reg32_t __reserved7[6];
    reg32_t FLASH_OTP_DATA3;
    reg32_t ANA2_TRIM;
    reg32_t LDO_TRIM;
    reg32_t __reserved8[0xB];
    reg32_t MAC_BLE_0;
    reg32_t MAC_BLE_1;
    reg32_t MAC_15_4_0;
    reg32_t MAC_15_4_1;
    reg32_t __reserved9[4];
    reg32_t FLASH_OTP_DATA4;
    reg32_t MISC_TRIM;
    reg32_t RCOSC_HF_TEMPCOMP;
    reg32_t __reserved10;
    reg32_t ICEPICK_DEVICE_ID;
    reg32_t FCFG1_REVISION;
    reg32_t MISC_OTP_DATA;
    reg32_t __reserved11[8];
    reg32_t IOCONF;
    reg32_t __reserved12;
    reg32_t CONFIG_IF_ADC;
    reg32_t CONFIG_OSC_TOP;
    reg32_t CONFIG_RF_FRONTEND;
    reg32_t CONFIG_SYNTH;
    reg32_t SOC_ADC_ABS_GAIN;
    reg32_t SOC_ADC_REL_GAIN;
    reg32_t __reserved13;
    reg32_t SOC_ADC_OFFSET_INT;
    reg32_t SOC_ADC_REF_TRIM_AND_OFFSET_E;
    reg32_t XT;
    reg32_t AMPCOMP_TH1;
    reg32_t AMPCOMP_TH2;
    reg32_t AMPCOMP_CTRL1;
    reg32_t ANABYPASS_VALUE2;
    reg32_t CONFIG_MISC_ADC;
    reg32_t __reserved14;
    reg32_t VOLT_TRIM;
    reg32_t OSC_CONF;
    reg32_t __reserved15;
    reg32_t VOLT_TRIM;
    reg32_t CAP_TRIM;
    reg32_t MISC_OTP_DATA_1;
    reg32_t PWD_CURR_20C;
    reg32_t PWD_CURR_35C;
    reg32_t PWD_CURR_50C;
    reg32_t PWD_CURR_65C;
    reg32_t PWD_CURR_80C;
    reg32_t PWD_CURR_95C;
    reg32_t PWD_CURR_110C;
    reg32_t PWD_CURR_125C;
} fcfg_regs_t;

#define FCFG ((fcfg_regs_t *) (FCFG_BASE))

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* CC26x0_FCFG_H */

/*@}*/
