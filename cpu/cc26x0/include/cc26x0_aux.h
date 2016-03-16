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
 * @brief           CC26x0 AUX register definitions
 */

#ifndef CC26x0_AUX_H
#define CC26x0_AUX_H

#include <stdbool.h>

#include "cc26x0.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    reg32_t GPIODOUT;
    reg32_t IOMODE;
    reg32_t GPIODIN;
    reg32_t GPIODOUTSET;
    reg32_t GPIODOUTCLR;
    reg32_t GPIODOUTTGL;
    reg32_t GPIODIE;
} aux_aiodio_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define AUX_AIODIO0_BASE      0x400C1000
#define AUX_AIODIO1_BASE      0x400C2000
/*@}*/

#define AUX_AIODIO0          ((aux_aiodio_regs_t *) (AUX_AIODIO0_BASE))
#define AUX_AIODIO1          ((aux_aiodio_regs_t *) (AUX_AIODIO1_BASE))


typedef struct {
    reg32_t CTL;
    reg32_t STAT;
    reg32_t RESULT;
    reg32_t SATCFG;
    reg32_t TRIGSRC;
    reg32_t TRIGCNT;
    reg32_t TRIGCNTLOAD;
    reg32_t TRIGCNTCFG;
    reg32_t PRECTL;
    reg32_t PRECNT;
} aux_tdc_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define AUX_TDC_BASE      0x400C4000
/*@}*/

#define AUX_TDC           ((aux_tdc_regs_t *) (AUX_TDC_BASE))


typedef struct {
    reg32_t VECCFG0;
    reg32_t VECCFG1;
    reg32_t SCEWEVSEL;
    reg32_t EVTOAONFLAGS;
    reg32_t EVTOAONPOL;
    reg32_t DMACTL;
    reg32_t SWEVSET;
    reg32_t EVSTAT0;
    reg32_t EVSTAT1;
    reg32_t EVTOMCUPOL;
    reg32_t EVTOMCUFLAGS;
    reg32_t COMBEVTOMCUMASK;
    reg32_t __reserved;
    reg32_t VECFLAGS;
    reg32_t EVTOMCUFLAGSCLR;
    reg32_t EVTOAONFLAGSCLR;
    reg32_t VECFLAGSCLR;
} aux_evtcl_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define AUX_EVCTL_BASE      0x400C5000
/*@}*/

#define AUX_EVCTL           ((aux_evctl_regs_t *) (AUX_EVCTL_BASE))


typedef struct {
    reg32_t MODCLKEN0;
    reg32_t PWROFFREQ;
    reg32_t PWRDWNREQ;
    reg32_t PWRDWNACK;
    reg32_t CLKLFREQ;
    reg32_t CLKLFACK;
    reg32_t __reserved1[4];
    reg32_t WUEVFLAGS;
    reg32_t WUEVCLR;
    reg32_t ADCCLKCTL;
    reg32_t TDCCLKCTL;
    reg32_t REFCLKCTL;
    reg32_t RTCSUBSECINC0;
    reg32_t RTCSUBSECINC1;
    reg32_t RTCSUBSECINCCTL;
    reg32_t MCUBUSCTL;
    reg32_t MCUBUSSTAT;
    reg32_t AONCTLSTAT;
    reg32_t AUXIOLATCH;
    reg32_t MODCLKEN1;
} aux_wuc_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define AUX_WUC_BASE      0x400C6000
/*@}*/

#define AUX_WUC           ((aux_wuc_regs_t *) (AUX_WUC_BASE))


typedef struct {
    reg32_t T0CFG;
    reg32_t T1CFG;
    reg32_t T0CTL;
    reg32_t T0TARGET;
    reg32_t T1TARGET;
    reg32_t T1CTL;
} aux_timer_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define AUX_TIMER_BASE      0x400C7000
/*@}*/

#define AUX_TIMER           ((aux_timer_regs_t *) (AUX_TIMER_BASE))


typedef struct {
    reg32_t SMPH0;
    reg32_t SMPH1;
    reg32_t SMPH2;
    reg32_t SMPH3;
    reg32_t SMPH4;
    reg32_t SMPH5;
    reg32_t SMPH6;
    reg32_t SMPH7;
    reg32_t AUTOTAKE;
} aux_smph_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define AUX_SMPH_BASE               0x400C8000
/*@}*/

#define AUX_SMPH ((aux_smph_regs_t *) (AUX_SMPH_BASE))


typedef struct {
    reg32_t __reserved[4];
    reg32_t ADCCTL;
    reg32_t ADCFIFOSTAT;
    reg32_t ADCFIFO;
    reg32_t ADCTRIG;
    reg32_t ISRCCTL;
} aux_anaif_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define AUX_ANAIF_BASE      0x400C9000
/*@}*/

#define AUX_ANAIF           ((aux_anaif_regs_t *) (AUX_ANAIF_BASE))


typedef struct {
    reg8_t MUX0;
    reg8_t MUX1;
    reg8_t MUX2;
    reg8_t MUX3;
    reg8_t ISRC;
    reg8_t COMP;
    reg8_t MUX4;
    reg8_t ADC0;
    reg8_t ADC1;
    reg8_t ADCREF0;
    reg8_t ADCREF1;
} adi_4_aux_regs_t;

/** @addtogroup cpu_specific_peripheral_memory_map
  * @{
  */
#define ADI_4_AUX_BASE      0x400CB000
/*@}*/

#define ADI_4_AUX           ((adi_4_aux_regs_t *) (ADI_4_AUX_BASE))


#define ADDI_SEM AUX_SMPH->SMPH0

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* CC26x0_AUX_H */

/*@}*/
