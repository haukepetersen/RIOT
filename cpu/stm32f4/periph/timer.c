/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_stm32f4
 * @{
 *
 * @file
 * @brief       Low-level timer driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdlib.h>

#include "cpu.h"
#include "board.h"
#include "sched.h"
#include "thread.h"
#include "periph_conf.h"
#include "periph/timer.h"

/**
 * @brief   Timer interrupt context
 */
timer_isr_ctx_t isr_ctx[TIMER_NUMOF];

static inline TIM_TypeDef *tim(tim_t dev)
{
    return timer_config[dev].dev;
}

static uint32_t *rcc_reg(tim_t dev)
{
    TIM_TypeDef *d = tim(dev);
    if ((d == TIM1) || (d == TIM8) || (d == TIM10) || (d == TIM11)) {
        return (uint32_t *)RCC->APB1ENR;
    }
    else {
        return (uint32_t *)RCC->APB2ENR;
    }
}

int timer_init(tim_t dev, unsigned int ticks_per_us, void (*callback)(int))
{
    /* make sure given device is valid */
    if (dev >= TIMER_NUMOF) {
        return -1;
    }

    /* save context */
    isr_ctx[dev].cb = callback;

    /* enable the timers clock */
    *rcc_reg(dev) |= (1 << timer_config[dev].rcc_bit);

    /* set timer to run in counter mode */
    tim(dev)->CR1 = 0;
    tim(dev)->CR2 = 0;
    /* set auto-reload and prescaler values and load new values */
    tim(dev)->EGR = TIM_EGR_UG;

    /* enable the timer's interrupt */
    timer_irq_enable(dev);
    /* start the timer */
    timer_start(dev);

    printf("rcc reg is %p\n", rcc_reg(dev));
    printf("rcc status 0x%08x\n", (unsigned)*rcc_reg(dev));
    puts("timer init ok");
    return 0;
}

int timer_set(tim_t dev, int channel, unsigned int timeout)
{
    int now = timer_read(dev);
    return timer_set_absolute(dev, channel, now + timeout - 1);
}

int timer_set_absolute(tim_t dev, int channel, unsigned int value)
{
    if (channel >= 4) {
        return -1;
    }

    printf("set timer %i ch %i to %i\n", (int)dev, channel, (int)value);

    tim(dev)->CCR[channel] = value;
    tim(dev)->SR &= ~(TIM_SR_CC1IF << channel);
    tim(dev)->DIER |= (TIM_DIER_CC1IE << channel);
    return 0;
}

int timer_clear(tim_t dev, int channel)
{
    if (channel >= 4) {
        return -1;
    }

    tim(dev)->DIER &= ~(TIM_DIER_CC1IE << channel);
    return 0;
}

unsigned int timer_read(tim_t dev)
{
    return (unsigned int)tim(dev)->CNT;
}

void timer_start(tim_t dev)
{
    tim(dev)->CR1 |= TIM_CR1_CEN;
}

void timer_stop(tim_t dev)
{
    tim(dev)->CR1 &= ~(TIM_CR1_CEN);
}

void timer_irq_enable(tim_t dev)
{
    NVIC_EnableIRQ(timer_config[dev].irqn);
}

void timer_irq_disable(tim_t dev)
{
    NVIC_DisableIRQ(timer_config[dev].irqn);
}

static inline void irq_handler(tim_t dev)
{
    if (tim(dev)->SR & TIM_SR_CC1IF) {
        tim(dev)->DIER &= ~TIM_DIER_CC1IE;
        tim(dev)->SR &= ~TIM_SR_CC1IF;
        isr_ctx[dev].cb(0);
    }
    else if (tim(dev)->SR & TIM_SR_CC2IF) {
        tim(dev)->DIER &= ~TIM_DIER_CC2IE;
        tim(dev)->SR &= ~TIM_SR_CC2IF;
        isr_ctx[dev].cb(1);
    }
    else if (tim(dev)->SR & TIM_SR_CC3IF) {
        tim(dev)->DIER &= ~TIM_DIER_CC3IE;
        tim(dev)->SR &= ~TIM_SR_CC3IF;
        isr_ctx[dev].cb(2);
    }
    else if (tim(dev)->SR & TIM_SR_CC4IF) {
        tim(dev)->DIER &= ~TIM_DIER_CC4IE;
        tim(dev)->SR &= ~TIM_SR_CC4IF;
        isr_ctx[dev].cb(3);
    }
    if (sched_context_switch_request) {
        thread_yield();
    }
}

void TIMER_0_ISR(void)
{
    irq_handler(0);
}

void TIMER_1_ISR(void)
{
    irq_handler(1);
}
