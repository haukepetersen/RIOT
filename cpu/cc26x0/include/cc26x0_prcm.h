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
 * @brief           CC26x0 PRCM register definitions
 */

#ifndef CC26x0_PRCM_H
#define CC26x0_PRCM_H

#include <cc26x0.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    reg32_t CTL0;
    reg32_t CTL1;
    reg32_t RADCEXTCFG;
    reg32_t AMPCOMPCTL;
    reg32_t AMPCOMPTH1;
    reg32_t AMPCOMPTH2;
    reg32_t ANABYPASSVAL1;
    reg32_t ANABYPASSVAL2;
    reg32_t ATESTCTL;
    reg32_t ADCDOUBLERNANOAMPCTL;
    reg32_t XOSCHFCTL;
    reg32_t LFOSCCTL;
    reg32_t RCOSCHFCTL;
    reg32_t STAT0;
    reg32_t STAT1;
    reg32_t STAT2;
} ddi0_osc_regs_t;

#define DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL              0x1 /* HF XOSC (RCOSC elsewise) */
#define DDI_0_OSC_CTL0_SCLK_MF_SRC_SEL              0x2
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_mask         0x6
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_HF_RCOSC     0x0
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_HF_XOSC      0x4
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_LF_RCOSC     0x8
#define DDI_0_OSC_CTL0_SCLK_LF_SRC_SEL_LF_XOSC      0xC
#define DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_RCOSC_mask  0x60
#define DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_RCOSC_HF    0x00 /* 31.25kHz */
#define DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_XOSC_HF     0x20 /* 31.25kHz */
#define DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_RCOSC_LF    0x40 /* 32kHz */
#define DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_XOSC_LF     0x60 /* 32.768kHz */
#define DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_RCOSC_mask  0x180
#define DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_RCOSC_HF    0x000 /* 48MHz */
#define DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_RCOSC_LF    0x080 /* 48MHz */
#define DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_XOSC_HF     0x100 /* 24MHz */
#define DDI_0_OSC_CTL0_CLK_LOSS_EN                  0x200 /* enable clock loss detection */
#define DDI_0_OSC_CTL0_XOSC_LF_DIG_BYPASS           0x400 /* bypass XOSC_LF and use digital input clock from AON foor xosx_lf (precuations in datasheet) */
#define DDI_0_OSC_CTL0_XOSC_HF_POWER_MODE           0x800
#define DDI_0_OSC_CTL0_RCOSC_LF_TRIMMED             0x1000
#define DDI_0_OSC_CTL0_ALLOW_SCLK_HF_SWITCHING      0x10000
#define DDI_0_OSC_CTL0_FORCE_KICKSTART_EN           0x400000
#define DDI_0_OSC_CTL0_DOUBLER_RESET_DURATION       0x2000000
#define DDI_0_OSC_CTL0_DOUBLER_START_DURATION_mask  0x6000000
#define DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL     0x10000000
#define DDI_0_OSC_CTL0_BYPASS_XOSC_LF_CLK_QUAL      0x20000000
#define DDI_0_OSC_CTL0_XTAL_IS_24M                  0x80000000

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define DDI0_OSC_BASE       0x400CA000
/*@}*/

#define DDI_0_OSC ((ddi0_osc_regs_t *) (DDI0_OSC_BASE))


typedef struct {
    reg32_t PWRCTL;
    reg32_t RESETCTL;
    reg32_t SLEEPCTL;
} aon_sysctl_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define AON_SYSCTL_BASE       0x40090000
/*@}*/

#define AON_SYSCTL ((aon_sysctl_regs_t *) (AON_SYSCTL_BASE))


typedef struct {
    reg32_t MCUCLK;
    reg32_t AUXCLK;
    reg32_t MCUCFG;
    reg32_t AUXCFG;
    reg32_t AUXCTL;
    reg32_t PWRSTAT;
    reg32_t __reserved1;
    reg32_t SHUTDOWN;
    reg32_t CTL0;
    reg32_t CTL1;
    reg32_t __reserved2[2];
    reg32_t RECHARGECFG;
    reg32_t RECHARGESTAT;
    reg32_t __reserved3;
    reg32_t OSCCFG;
    reg32_t JTAGCFG;
    reg32_t JTAGUSERCODE;
} aon_wuc_regs_t;

#define MCUCLK_PWR_DWN_SRC          0x1 /* SCLK_LF in powerdown (no clock elsewise) */
#define MCUCLK_PWR_DWN_SRC_mask     0x3
#define MCUCLK_RCOSC_HF_CAL_DONE    0x4 /* set by MCU bootcode. RCOSC_HF is calibrated to 48 MHz, allowing FLASH to power up */

#define AUXCLK_SRC_HF               0x1 /* SCLK for AUX */
#define AUXCLK_SRC_LF               0x4
#define AUXCLK_SRC_mask             0x7 /* garuanteed to be glitchless */
#define AUXCLK_SCLK_HF_DIV_pos      8 /* don't set while SCLK_HF active for AUX */
#define AUXCLK_SCLK_HF_DIV_mask     0x700 /* divisor will be 2^(value+1) */
#define AUXCLK_PWR_DWN_SRC_pos      11 /* SCLK_LF in powerdown when SCLK_HF is source (no clock elsewise?!)  */
#define AUXCLK_PWR_DWN_SRC_mask     0x1800 /* datasheet is confusing.. */

#define MCUCFG_SRAM_RET_OFF         0x0 /* no retention for any SRAM-bank */
#define MCUCFG_SRAM_RET_B0          0x1
#define MCUCFG_SRAM_RET_B01         0x3
#define MCUCFG_SRAM_RET_B012        0x7
#define MCUCFG_SRAM_RET_B0124       0xF /* retention for banks 0, 1, 2, and 3 */
#define MCUCFG_SRAM_FIXED_WU_EN     0x100
#define MCUCFG_SRAM_VIRT_OFF        0x200

#define AUXCFG_RAM_RET_EN           0x1 /* retention for AUX_RAM bank 0. is off when otherwise in retention mode */

#define AUXCTL_AUX_FORCE_ON     0x1
#define AUXCTL_SWEV             0x2
#define AUXCTL_SCE_RUN_EN       0x3
#define AUXCTL_RESET_REQ        0x80000000

#define PWRSTAT_AUX_RESET_DONE      0x2
#define PWRSTAT_AUX_BUS_CONNECTED   0x4
#define PWRSTAT_MCU_PD_ON           0x10
#define PWRSTAT_AUX_PD_ON           0x20
#define PWRSTAT_JTAG_PD_ON          0x40
#define PWRSTAT_AUX_PWR_DNW         0x200

#define SHUTDOWN_EN     0x1 /* register/cancel shutdown request */

#define AONWUC_CTL0_MCU_SRAM_ERASE      0x4
#define AONWUC_CTL0_AUX_SRAM_ERASE      0x8
#define AONWUC_CTL0_PWR_DWN_DIS         0x10 /* disable powerdown on request */

#define AONWUC_CTL1_MCU_WARM_RESET      0x1 /* last MCU reset was a warm reset */
#define AONWUC_CTL1_MCU_RESET_SRC       0x2 /* JTAG was source of last reset (MCU SW elsewise) */

#define RECHARGECFG_PER_E_mask          0x00000007 /* number of 32KHz clocks between activation of recharge controller: */
#define RECHARGECFG_PER_M_mask          0x000000F8 /* computed as follows: PERIOD = (PER_M*16+15) * 2^(PER_E) */
#define RECHARGECFG_MAX_PER_E_mask      0x00000700 /* maximum period the recharge algorithm can take */
#define RECHARGECFG_MAX_PER_M_mask      0x0000F800 /* computed as follows: MAXCYCLES = (MAX_PER_M*16+15) * 2^(MAX_PER_E) */
#define RECHARGECFG_C1_mask             0x000F0000 /* i resign */
#define RECHARGECFG_C2_mask             0x000F0000
#define RECHARGECFG_ADAPTIVE_EN         0x80000000

#define RECHARGESTAT_MAX_USED_PER_mask  0x0FFFF
#define RECHARGESTAT_VDDR_SMPLS_mask    0xF0000

#define OSCCFG_PER_E_mask               0x07 /* number of 32KHz clocks between oscillator amplitude callibrations */
#define OSCCFG_PER_M_mask               0xF8 /* computed as follows: PERIOD = (PER_M*16+15) * 2^(PER_E) */

#define JTAGCFG_JTAG_PD_FORCE_ON        0x10

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define AON_WUC_BASE       0x40091000
/*@}*/

#define AON_WUC ((aon_wuc_regs_t *) (AON_WUC_BASE))


typedef struct {
    reg32_t INFRCLKDIVR;
    reg32_t INFRCLKDIVS;
    reg32_t INFRCLKDIVDS;
    reg32_t VDCTL;
    reg32_t __reserved1[6];
    reg32_t CLKLOADCTL;
    reg32_t RFCCLKG;
    reg32_t VIMSCLKG;
    reg32_t __reserved2[2];
    reg32_t SECDMACLKGR;
    reg32_t SECDMACLKGS;
    reg32_t SECDMACLKGDS;
    reg32_t GPIOCLKGR;
    reg32_t GPIOCLKGS;
    reg32_t GPIOCLKGDS;
    reg32_t GPTCLKGR;
    reg32_t GPTCLKGS;
    reg32_t GPTCLKGDS;
    reg32_t I2CCLKGR;
    reg32_t I2CCLKGS;
    reg32_t I2CCLKGDS;
    reg32_t UARTCLKGR;
    reg32_t UARTCLKGS;
    reg32_t UARTCLKGDS;
    reg32_t SSICLKGR;
    reg32_t SSICLKGS;
    reg32_t SSICLKGDS;
    reg32_t I2SCLKGR;
    reg32_t I2SCLKGS;
    reg32_t I2SCLKGDS;
    reg32_t __reserved3[10];
    reg32_t CPUCLKDIV;
    reg32_t __reserved4[3];
    reg32_t I2SBCLKSEL;
    reg32_t GPTCLKDIV;
    reg32_t I2SCLKCTL;
    reg32_t I2SMCLKDIV;
    reg32_t I2SBCLKDIV;
    reg32_t I2SWCLKDIV;
    reg32_t __reserved5[11];
    reg32_t SWRESET;
    reg32_t WARMRESET;
    reg32_t __reserved6[6];
    reg32_t PDCTL0;
    reg32_t PDCTL0RFC;
    reg32_t PDCTL0SERIAL;
    reg32_t PDCTL0PERIPH;
    reg32_t __reserved7;
    reg32_t PDSTAT0;
    reg32_t PDSTAT0RFC;
    reg32_t PDSTAT0SERIAL;
    reg32_t PDSTAT0PERIPH;
    reg32_t __reserved8[11];
    reg32_t PDCTL1;
    reg32_t __reserved9;
    reg32_t PDCTL1CPU;
    reg32_t PDCTL1RFC;
    reg32_t PDCTL1VIMS;
    reg32_t __reserved10;
    reg32_t PDSTAT1;
    reg32_t PDSTAT1BUS;
    reg32_t PDSTAT1RFC;
    reg32_t PDSTAT1CPU;
    reg32_t PDSTAT1VIMS;
    reg32_t __reserved11[10];
    reg32_t RFCMODESEL;
    reg32_t __reserved12[20];
    reg32_t RAMRETEN;
    reg32_t __reserved13;
    reg32_t PDRETEN;
    reg32_t __reserved14[8];
    reg32_t RAMHWOPT;
} prcm_regs_t;

#define CLKLOADCTL_LOAD        0x1
#define CLKLOADCTL_LOADDONE    0x2

#define PDCTL0_RFC_ON       0x1
#define PDCTL0_SERIAL_ON    0x2
#define PDCTL0_PERIPH_ON    0x4

#define PDSTAT0_RFC_ON       0x1
#define PDSTAT0_SERIAL_ON    0x2
#define PDSTAT0_PERIPH_ON    0x4

#define PDCTL1_CPU_ON       0x2
#define PDCTL1_RFC_ON       0x4
#define PDCTL1_VIMS_ON      0x8

#define PDSTAT1_CPU_ON      0x2
#define PDSTAT1_RFC_ON      0x4
#define PDSTAT1_VIMS_ON     0x8

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define PRCM_BASE       0x40082000
/*@}*/

#define PRCM ((prcm_regs_t *) (PRCM_BASE))

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* CC26x0_PRCM_H */

/*@}*/
