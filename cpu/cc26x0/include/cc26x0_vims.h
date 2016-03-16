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
 * @brief           CC26x0 VIMS register definitions
 */

#ifndef CC26x0_VIMS_H
#define CC26x0_VIMS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    reg32_t __reserved1[7];
    reg32_t STAT;
    reg32_t CTL;
    reg32_t __reserved2;
    reg32_t SYSCODE_START;
    reg32_t FLASH_SIZE;
    reg32_t __reserved3[3];
    reg32_t FWLOCK;
    reg32_t FWFLAG;
    reg32_t __reserved4[0x3EF];
    reg32_t EFUSE;
    reg32_t EFUSEADDR;
    reg32_t DATAUPPER;
    reg32_t DATALOWER;
    reg32_t EFUSECFG;
    reg32_t EFUSESTAT;
    reg32_t ACC;
    reg32_t BOUNDARY;
    reg32_t EFUSEFLAG;
    reg32_t EFUSEKEY;
    reg32_t EFUSERELEASE;
    reg32_t EFUSEPINS;
    reg32_t EFUSECRA;
    reg32_t EFUSEREAD;
    reg32_t EFUSEPROGRAM;
    reg32_t EFUSEERROR;
    reg32_t SINGLEBIT;
    reg32_t TWOBIT;
    reg32_t SELFTESTCYC;
    reg32_t SELFTESTSIGN;
    reg32_t __reserved5[0x3ec];
    reg32_t FRDCTL;
    reg32_t FSPRD;
    reg32_t FEDACCTL1;
    reg32_t __reserved6[4];
    reg32_t FEDACSTAT;
    reg32_t __reserved7[4];
    reg32_t FBPROT;
    reg32_t FBSE;
    reg32_t FBBUSY;
    reg32_t FBAC;
    reg32_t FBFALLBACK;
    reg32_t FBPRDY;
    reg32_t FPAC1;
    reg32_t FPAC2;
    reg32_t FMAC;
    reg32_t FMSTAT;
    reg32_t __reserved8[3];
    reg32_t FLOCK;
    reg32_t __reserved9[6];
    reg32_t FVREADCT;
    reg32_t FVHVCT1;
    reg32_t FVHVCT2;
    reg32_t FVHVCT3;
    reg32_t FVNVCT;
    reg32_t FVSLP;
    reg32_t FVWLCT;
    reg32_t FEFUSECTL;
    reg32_t FEFUSESTAT;
    reg32_t FEFUSEDATA;
    reg32_t FSEQPMP;
    reg32_t __reserved10[21];
    reg32_t FBSTROBES;
    reg32_t FPSTROBES;
    reg32_t FBMODE;
    reg32_t FTCR;
    reg32_t FADDR;
    reg32_t __reserved11[2];
    reg32_t FTCTL;
    reg32_t FWPWRITE0;
    reg32_t FWPWRITE1;
    reg32_t FWPWRITE2;
    reg32_t FWPWRITE3;
    reg32_t FWPWRITE4;
    reg32_t FWPWRITE5;
    reg32_t FWPWRITE6;
    reg32_t FWPWRITE7;
    reg32_t FWPWRITE_ECC;
    reg32_t FSWSTAT;
    reg32_t __reserved12[0x2E];
    reg32_t FSM_GLBCTL;
    reg32_t FSM_STATE;
    reg32_t FSM_STAT;
    reg32_t FSM_CMD;
    reg32_t FSM_PE_OSU;
    reg32_t FSM_VSTAT;
    reg32_t FSM_PE_VSU;
    reg32_t FSM_CMP_VSU;
    reg32_t FSM_EX_VAL;
    reg32_t FSM_RD_H;
    reg32_t FSM_P_OH;
    reg32_t FSM_ERA_OH;
    reg32_t FSM_SAV_PPUL;
    reg32_t FSM_PE_VH;
    reg32_t __reserved13[2];
    reg32_t FSM_PRG_PW;
    reg32_t FSM_ERA_PW;
    reg32_t __reserved14[3];
    reg32_t FSM_SAV_ERA_PUL;
    reg32_t FSM_TIMER;
    reg32_t FSM_MODE;
    reg32_t FSM_PGM;
    reg32_t FSM_ERA;
    reg32_t FSM_PRG_PUL;
    reg32_t FSM_ERA_PUL;
    reg32_t FSM_STEP_SIZE;
    reg32_t FSM_PUL_CNTR;
    reg32_t FSM_EC_STEP_HEIGHT;
    reg32_t FSM_ST_MACHINE;
    reg32_t FSM_FLES;
    reg32_t __reserved15;
    reg32_t FSM_WR_ENA;
    reg32_t FSM_ACC_PP;
    reg32_t FSM_ACC_EP;
    reg32_t __reserved16[3];
    reg32_t FSM_ADDR;
    reg32_t FSM_SECTOR;
    reg32_t FMC_REV_ID;
    reg32_t FSM_ERR_ADDR;
    reg32_t FSM_PGM_MAXPUL;
    reg32_t FSM_EXECUTE;
    reg32_t __reserved17[2];
    reg32_t FSM_SECTOR1;
    reg32_t FSM_SECTOR2;
    reg32_t __reserved18[6];
    reg32_t FSM_BSLE0;
    reg32_t FSM_BSLE1;
    reg32_t __reserved19[2];
    reg32_t FSM_BSLP0;
    reg32_t FSM_BSLP1;
    reg32_t __reserved20[0x42];
    reg32_t FCFG_BANK;
    reg32_t FCFG_WRAPPER;
    reg32_t FCFG_BNK_TYPE;
    reg32_t __reserved21;
    reg32_t FCFG_B0_START;
    reg32_t FCFG_B1_START;
    reg32_t FCFG_B2_START;
    reg32_t FCFG_B3_START;
    reg32_t FCFG_B4_START;
    reg32_t FCFG_B5_START;
    reg32_t FCFG_B6_START;
    reg32_t FCFG_B7_START;
    reg32_t FCFG_B0_SSIZE0;
} flash_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define FLASH_BASEADDR                           0x40030000 /**< base address of FLASH memory */
/*@}*/

#define FLASH                                ((flash_regs_t *)(FLASH_BASEADDR + 0x4))


typedef struct {
    reg32_t STAT;
    reg32_t CTL;
} vims_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define VIMS_BASE                           0x40034000 /**< base address of VIMS memory */
/*@}*/

#define VIMS                                ((vims_regs_t *)(VIMS_BASE + 0x4))

#define VIMS_CTL_STATS_CLR                  0x80000000
#define VIMS_CTL_STATS_CLR_m                0x80000000

#define VIMS_CTL_STATS_EN                   0x40000000
#define VIMS_CTL_STATS_EN_m                 0x40000000

#define VIMS_CTL_DYN_CG_EN                  0x20000000
#define VIMS_CTL_DYN_CG_EN_m                0x20000000

#define VIMS_CTL_IDCODE_LB_DIS              0x00000020
#define VIMS_CTL_IDCODE_LB_DIS_m            0x00000020

#define VIMS_CTL_SYSBUS_LB_DIS              0x00000010
#define VIMS_CTL_SYSBUS_LB_DIS_m            0x00000010

#define VIMS_CTL_ARB_CFG                    0x00000008
#define VIMS_CTL_ARB_CFG_m                  0x00000008

#define VIMS_CTL_PREF_EN                    0x00000004
#define VIMS_CTL_PREF_EN_m                  0x00000004

#define VIMS_CTL_MODE_GPRAM                 0x00000000
#define VIMS_CTL_MODE_CACHE                 0x00000001
#define VIMS_CTL_MODE_SPLIT                 0x00000002
#define VIMS_CTL_MODE_OFF                   0x00000003
#define VIMS_CTL_MODE_m                     0x00000003

#ifdef __cplusplus
}
#endif

#endif /* CC26x0_VIMS_H */

/*@}*/
