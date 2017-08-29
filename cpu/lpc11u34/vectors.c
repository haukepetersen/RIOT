/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_lpc11u34
 * @{
 *
 * @file
 * @brief       Startup code and interrupt vector definition
 *
 * @author      Paul RATHGEB <paul.rathgeb@skynet.be>
 *
 * @}
 */

#include <stdint.h>
#include "vectors_cortexm.h"

/* get the start of the ISR stack as defined in the linkerscript */
extern uint32_t _estack;

/* define a local dummy handler as it needs to be in the same compilation unit
 * as the alias definition */
void dummy_handler(void) {
    dummy_handler_default();
}

/* LPC11U34 specific interrupt vector */
WEAK_DEFAULT void isr_pinint0(void);
WEAK_DEFAULT void isr_pinint1(void);
WEAK_DEFAULT void isr_pinint2(void);
WEAK_DEFAULT void isr_pinint3(void);
WEAK_DEFAULT void isr_pinint4(void);
WEAK_DEFAULT void isr_pinint5(void);
WEAK_DEFAULT void isr_pinint6(void);
WEAK_DEFAULT void isr_pinint7(void);
WEAK_DEFAULT void isr_gint0(void);
WEAK_DEFAULT void isr_gint1(void);
WEAK_DEFAULT void isr_ssp1(void);
WEAK_DEFAULT void isr_i2c0(void);
WEAK_DEFAULT void isr_ct16b0(void);
WEAK_DEFAULT void isr_ct16b1(void);
WEAK_DEFAULT void isr_ct32b0(void);
WEAK_DEFAULT void isr_ct32b1(void);
WEAK_DEFAULT void isr_ssp0(void);
WEAK_DEFAULT void isr_usart0(void);
WEAK_DEFAULT void isr_usb_irq(void);
WEAK_DEFAULT void isr_usb_fiq(void);
WEAK_DEFAULT void isr_adc(void);
WEAK_DEFAULT void isr_wwdt(void);
WEAK_DEFAULT void isr_bod(void);
WEAK_DEFAULT void isr_flash(void);
WEAK_DEFAULT void isr_usb_wakeup(void);

/* CPU specific interrupt vector table */
ISR_VECTOR(1) const isr_t vector_cpu[] = {
    isr_pinint0,            /* Pin ISR0 */
    isr_pinint1,            /* Pin ISR1 */
    isr_pinint2,            /* Pin ISR2 */
    isr_pinint3,            /* Pin ISR3 */
    isr_pinint4,            /* Pin ISR4 */
    isr_pinint5,            /* Pin ISR5 */
    isr_pinint6,            /* Pin ISR6 */
    isr_pinint7,            /* Pin ISR7 */
    isr_gint0,              /* GPIO Group ISR 0 */
    isr_gint1,              /* GPIO Group ISR 1 */
    (0UL),                  /* Reserved */
    (0UL),                  /* Reserved */
    (0UL),                  /* Reserved */
    (0UL),                  /* Reserved */
    isr_ssp1,               /* ssp1 */
    isr_i2c0,               /* i2c0 */
    isr_ct16b0,             /* ct16b0 */
    isr_ct16b1,             /* ct16b1 */
    isr_ct32b0,             /* ct32b0 */
    isr_ct32b1,             /* ct32b1 */
    isr_ssp0,               /* ssp0 */
    isr_usart0,             /* usart0 */
    isr_usb_irq,            /* USB */
    isr_usb_fiq,            /* USB */
    isr_adc,                /* ADC */
    isr_wwdt,               /* windowed watchdog */
    isr_bod,                /* brown out */
    isr_flash,              /* brown out detect */
    (0UL),                  /* Reserved */
    (0UL),                  /* Reserved */
    isr_usb_wakeup,         /* flash */
    (0UL),                  /* Reserved */
};
