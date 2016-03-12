/*
 * Copyright (C) 2016 Leon George
 * (who won't go after you should you copy any part of this file ;-)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */
/**
 * @ingroup     driver_periph
 * @{
 *
 * @file
 * @brief       low-level timer driver implementation for the CC26x0 CPU
 *
 * @author      Leon M. George <leon@georgemail.eu>
 *
 * @}
 */

#include <stdlib.h>
#include <stdio.h>

#include "board.h"
#include "cpu.h"
#include "sched.h"
#include "thread.h"
#include "periph/timer.h"
#include "periph_conf.h"

#include "cc26x0-prcm.h"

timer_isr_ctx_t config[TIMER_NUMOF];

static gpt_reg_t *select_gpt(tim_t dev)
{
    return timer_conf[dev].gpt;
}

int timer_init(tim_t dev, unsigned long freq, timer_cb_t cb, void *arg)
{
    gpt_reg_t *gpt = select_gpt(dev);

    unsigned int gpt_num = ((uintptr_t) gpt - (uintptr_t) GPT0_BASE) / 0x1000;

    config[dev].cb = cb;
    config[dev].arg = arg;

    PRCM->GPTCLKGR |= (1 << gpt_num);
    PRCM->GPTCLKGS |= (1 << gpt_num);
    PRCM->CLKLOADCTL = CLKLOADCTL_LOAD;
    while (!(PRCM->CLKLOADCTL & CLKLOADCTL_LOADDONE)) ;

    gpt->CTL = 0;

    gpt->CFG  = GPT_CFG_16T;
    gpt->TAMR = GPT_TXMR_TXMR_PERIODIC | GPT_TXMR_TXCDIR_UP | GPT_TXMR_TXMIE;

    // oh crap. this seems to be ignored in concatenated mode
    gpt->TAPR = (RCOSC48M_FREQ / freq) - 1;

    timer_irq_enable(dev);

    gpt->CTL |= GPT_CTL_TAEN;

    return 0;
}

int timer_set(tim_t dev, int channel, unsigned int timeout)
{
    return timer_set_absolute(dev, channel, timer_read(dev) + timeout);
}

int timer_set_absolute(tim_t dev, int channel, unsigned int value)
{
    if (channel != 0)
        return -1;

    gpt_reg_t *gpt = timer_conf[dev].gpt;

    /* TODO this is a workaround because the prescaler doesn't work */
    gpt->TAMATCHR = value * gpt->TAPR;
    gpt->IMR |= GPT_IMR_TAMIM;

    return 1;
}

int timer_clear(tim_t dev, int channel)
{
    timer_conf[dev].gpt->CTL &= (~GPT_CTL_TAEN);

    return 1;
}

unsigned int timer_read(tim_t dev)
{
    /* TODO this is a workaround because the prescaler doesn't work */
    return timer_conf[dev].gpt->TAR / timer_conf[dev].gpt->TAPR;
}

void timer_stop(tim_t dev)
{
    timer_conf[dev].gpt->CTL &= (~GPT_CTL_TAEN);
}

void timer_start(tim_t dev)
{
    timer_conf[dev].gpt->CTL |= GPT_CTL_TAEN;
}

void timer_irq_enable(tim_t dev)
{
    NVIC_EnableIRQ(timer_conf[dev].irqn);
}

void timer_irq_disable(tim_t dev)
{
    NVIC_DisableIRQ(timer_conf[dev].irqn);
}

static void _interrupt(tim_t dev, int chan)
{
    uint32_t mask = GPT_IMR_TAMIM;

    uint32_t mis = timer_conf[dev].gpt->MIS;
    timer_conf[dev].gpt->ICLR = mis;

    if (mis & mask) {
        timer_conf[dev].gpt->IMR &= ~mask;

        if (config[dev].cb != NULL) config[dev].cb(config[dev].arg, chan);
    }

    if (sched_context_switch_request) {
        thread_yield();
    }
}

void TIMER_0_ISR_1(void)
{
    _interrupt(0, 0);
}

void TIMER_1_ISR_1(void)
{
    _interrupt(1, 0);
}

void TIMER_2_ISR_1(void)
{
    _interrupt(2, 0);
}

void TIMER_3_ISR_1(void)
{
    _interrupt(3, 0);
}
