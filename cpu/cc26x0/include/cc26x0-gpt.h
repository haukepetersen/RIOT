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
 * @brief           driver for the cc26x0 GPT controller
 *
 * @author          Leon George <leon@georgemail.eu>
 *
 * @defgroup cc26x0-gpt CC26x0 general-purpose timers
 * @{
 */

#ifndef CC26x0_GPT_H
#define CC26x0_GPT_H

#include "cc26x0.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup cpu_specific_peripheral_memory_map
 * @{
 */
#define GPT0_BASE               0x40010000
#define GPT1_BASE               0x40011000
#define GPT2_BASE               0x40012000
#define GPT3_BASE               0x40013000
/** @} */

typedef struct {
    reg32_t CFG;
    reg32_t TAMR;
    reg32_t TBMR;
    reg32_t CTL;
    reg32_t SYNC;
    reg32_t __reserved1;
    reg32_t IMR;
    reg32_t RIS;
    reg32_t MIS;
    reg32_t ICLR;
    reg32_t TAILR;
    reg32_t TBILR;
    reg32_t TAMATCHR;
    reg32_t TBMATCHR;
    reg32_t TAPR;
    reg32_t TBPR;
    reg32_t TAPMR;
    reg32_t TBPMR;
    reg32_t TAR;
    reg32_t TBR;
    reg32_t TAV;
    reg32_t TBV;
    reg32_t RTCPD;
    reg32_t TAPS;
    reg32_t TBPS;
    reg32_t TAPV;
    reg32_t TBPV;
    reg32_t DMAEV;
    reg32_t __reserved2[976];
    reg32_t VERSION;
    reg32_t ANDCCP;
} gpt_reg_t;

#define GPT0 ((gpt_reg_t *) (GPT0_BASE))
#define GPT1 ((gpt_reg_t *) (GPT1_BASE))
#define GPT2 ((gpt_reg_t *) (GPT2_BASE))
#define GPT3 ((gpt_reg_t *) (GPT3_BASE))

#define GPT_CFG_32T         0
#define GPT_CFG_32RTC       1
#define GPT_CFG_16T         4

#define GPT_TXMR_TXMR_ONE_SHOT          0x00000001
#define GPT_TXMR_TXMR_PERIODIC          0x00000002
#define GPT_TXMR_TXMR_CAPTURE           0x00000003
#define GPT_TXMR_TXCM_EDGECNT           0x00000000
#define GPT_TXMR_TXCM_EDGETIME          0x00000004
#define GPT_TXMR_TXAMS_CAPTCOMP         0x00000000
#define GPT_TXMR_TXAMS_PWM              0x00000008
#define GPT_TXMR_TXCDIR_DOWN            0x00000000
#define GPT_TXMR_TXCDIR_UP              0x00000010 /* starts from 0 */
#define GPT_TXMR_TXMIE                  0x00000020 /* match interrupt */
#define GPT_TXMR_TXWOT                  0x00000040 /* wait on trigger from daisy */
#define GPT_TXMR_TXSNAPS                0x00000080
#define GPT_TXMR_TXILD_CLOCK            0x00000000 /* interrupt loac: update TXPR or TXR */
#define GPT_TXMR_TXILD_TIMEOUT          0x00000100
#define GPT_TXMR_TXPWMIE                0x00000200
#define GPT_TXMR_TXMRSU                 0x00000400
#define GPT_TXMR_TXPLO                  0x00000800
#define GPT_TXMR_TXCIN                  0x00001000
#define GPT_TXMR_TCACT_DIS              0x00000000
#define GPT_TXMR_TCACT_TGL_TO           0x00002000
#define GPT_TXMR_TCACT_CLR_TO           0x00004000
#define GPT_TXMR_TCACT_SET_TO           0x00006000
#define GPT_TXMR_TCACT_SET_NOW_TGL_TO   0x00008000
#define GPT_TXMR_TCACT_CLR_NOW_TGL_TO   0x0000a000
#define GPT_TXMR_TCACT_SET_NOW_CLR_TO   0x0000c000
#define GPT_TXMR_TCACT_CLR_NOW_SET_TO   0x0000e000

#define GPT_CTL_TAEN                0x00000001
#define GPT_CTL_TASTALL             0x00000002
#define GPT_CTL_TAEVENT_POS         0x00000000
#define GPT_CTL_TAEVENT_NEG         0x00000004
#define GPT_CTL_TAEVENT_BOTH        0x0000000c
#define GPT_CTL_RTCEN               0x00000010
#define GPT_CTL_TAPWML_INV          0x00000040
#define GPT_CTL_TBEN                0x00000100 /* still need capture CFG */
#define GPT_CTL_TBSTALL             0x00000200
#define GPT_CTL_TBEVENT_POS         0x00000000
#define GPT_CTL_TBEVENT_NEG         0x00000400
#define GPT_CTL_TBEVENT_BOTH        0x00000c00
#define GPT_CTL_TBPWML_INV          0x00004000

#define GPT_SYNC_SYNC1_A            0x00000001
#define GPT_SYNC_SYNC1_B            0x00000002
#define GPT_SYNC_SYNC2_A            0x00000004
#define GPT_SYNC_SYNC2_B            0x00000008
#define GPT_SYNC_SYNC3_A            0x00000010
#define GPT_SYNC_SYNC3_B            0x00000020
#define GPT_SYNC_SYNC4_A            0x00000040
#define GPT_SYNC_SYNC4_B            0x00000080

#define GPT_IMR_TATOIM              0x00000001
#define GPT_IMR_CAMIM               0x00000002
#define GPT_IMR_CAEIM               0x00000004
#define GPT_IMR_RTCIM               0x00000008
#define GPT_IMR_TAMIM               0x00000010
#define GPT_IMR_DMAAIM              0x00000020
#define GPT_IMR_TBTOIM              0x00000100
#define GPT_IMR_CBMIM               0x00000200
#define GPT_IMR_CBEIM               0x00000400
#define GPT_IMR_TBMIM               0x00000800
#define GPT_IMR_DMABIM              0x00002000
#define GPT_IMR_WUMIS               0x00010000

#define GPT_RIS_TATORIS             0x00000001
#define GPT_RIS_CAMRIS              0x00000002
#define GPT_RIS_CAERIS              0x00000004
#define GPT_RIS_RTCRIS              0x00000008
#define GPT_RIS_TAMRIS              0x00000010
#define GPT_RIS_TBTORIS             0x00000100
#define GPT_RIS_CBMRIS              0x00000200
#define GPT_RIS_CBERIS              0x00000400
#define GPT_RIS_TBMRIS              0x00000800
#define GPT_RIS_DMARIS              0x00002000
#define GPT_RIS_WURIS               0x00010000

#define GPT_MIS_TATOMIS             0x00000001
#define GPT_MIS_CAMMIS              0x00000002
#define GPT_MIS_CAEMIS              0x00000004
#define GPT_MIS_RTCMIS              0x00000008
#define GPT_MIS_TAMMIS              0x00000010
#define GPT_MIS_TBTOMIS             0x00000100
#define GPT_MIS_CBMMIS              0x00000200
#define GPT_MIS_CBEMIS              0x00000400
#define GPT_MIS_TBMMIS              0x00000800
#define GPT_MIS_DMAMIS              0x00002000
#define GPT_MIS_WUMIS               0x00010000

#define GPT_ICLR_TATOCINT           0x00000001
#define GPT_ICLR_CAMCINT            0x00000002
#define GPT_ICLR_CAECINT            0x00000004
#define GPT_ICLR_RTCCINT            0x00000008
#define GPT_ICLR_TAMCINT            0x00000010
#define GPT_ICLR_TBTOCINT           0x00000100
#define GPT_ICLR_CBMCINT            0x00000200
#define GPT_ICLR_CBECINT            0x00000400
#define GPT_ICLR_TBMCINT            0x00000800
#define GPT_ICLR_DMACINT            0x00002000
#define GPT_ICLR_WUCINT             0x00010000

#define GPT_DMAEV_TATODMAEN         0x00000001
#define GPT_DMAEV_CAMDMAEN          0x00000002
#define GPT_DMAEV_CAEDMAEN          0x00000004
#define GPT_DMAEV_RTCDMAEN          0x00000008
#define GPT_DMAEV_TAMDMAEN          0x00000010
#define GPT_DMAEV_TBTODMAEN         0x00000100
#define GPT_DMAEV_CBMDMAEN          0x00000200
#define GPT_DMAEV_CBEDMAEN          0x00000400
#define GPT_DMAEV_TBMDMAEN          0x00000800

#define GPT_NUMOF               4
#define NUM_CHANNELS_PER_GPT    1

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* CC26x0_GPT_H */

/** @} */
