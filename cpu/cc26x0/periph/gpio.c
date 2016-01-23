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
 * @brief       low-level GPIO driver implementation
 *
 * @author      Leon M. George <leon@georgemail.eu>
 *
 * @}
 */

#include "cpu.h"
#include "sched.h"
#include "thread.h"
#include "periph/gpio.h"
#include "periph_conf.h"

#define GPIO_ISR_CHAN_NUMOF             (32)

typedef struct {
    void (*cb)(void *arg);
    void *arg;
} gpio_ctx_t;

static gpio_ctx_t gpio_chan[GPIO_ISR_CHAN_NUMOF];

int gpio_init(gpio_t pin, gpio_dir_t dir, gpio_pp_t pullup)
{
    if ((pin < 0) || (pin > 31))
        return -1;

    PRCM->PDCTL0 |= PDCTL0_PERIPH_ON;
    while(!(PRCM->PDSTAT0 & PDSTAT0_PERIPH_ON)) ;

    PRCM->GPIOCLKGR |= 1;
    PRCM->CLKLOADCTL |= CLKLOADCTL_LOAD;
    while (!(PRCM->CLKLOADCTL & CLKLOADCTL_LOADDONE)) ;

    IOC->CFG[pin] = 0;

    if (dir == GPIO_DIR_OUT) {
        GPIO->DOE |= (1 << pin);
        GPIO->DOUTCLR = (1 << pin);
    }
    else {
        IOC->CFG[pin] |= pullup;
        IOC->CFG[pin] |= IOCFG_INPUT_ENABLE;
    }
    return 0;
}

int gpio_init_int(gpio_t pin,
                   gpio_pp_t pullup, gpio_flank_t flank,
                   gpio_cb_t cb, void *arg)
{
    int init = gpio_init(pin, GPIO_DIR_IN, pullup);
    if (init != 0)
        return init;

    NVIC_EnableIRQ(EDGE_DETECT_IRQN);

    IOC->CFG[pin] |= flank;

    gpio_chan[pin].cb = cb;
    gpio_chan[pin].arg = arg;

    /* clears the interrupt flag */
    GPIO->EVFLAGS |= (1 << pin);

    gpio_irq_enable(pin);

    return 0;
}

void gpio_irq_enable(gpio_t pin)
{
    IOC->CFG[pin] |= IOCFG_EDGEIRQ_ENABLE;
}

void gpio_irq_disable(gpio_t pin)
{
    IOC->CFG[pin] &= ~IOCFG_EDGEIRQ_ENABLE;
}

int gpio_read(gpio_t pin)
{
    if (GPIO->DOE & (1 << pin)) {
        return (GPIO->DOUT >> pin) & 1;
    }
    else {
        return (GPIO->DIN >> pin) & 1;
    }
}

void gpio_set(gpio_t pin)
{
    GPIO->DOUTSET = (1 << pin);
}

void gpio_clear(gpio_t pin)
{
    GPIO->DOUTCLR = (1 << pin);
}

void gpio_toggle(gpio_t pin)
{
    GPIO->DOUTTGL = (1 << pin);
}

void gpio_write(gpio_t pin, int value)
{
    if (value) {
        GPIO->DOUTSET = (1 << pin);
    } else {
        GPIO->DOUTCLR = (1 << pin);
    }
}

void isr_edge(void)
{
    for (unsigned pin = 0; pin < GPIO_ISR_CHAN_NUMOF; pin++) {
        /* doc claims EVFLAGS will only be set for pins that have edge detection enabled */
        if (GPIO->EVFLAGS & (1 << pin)) {
            GPIO->EVFLAGS |= (1 << pin);
            gpio_chan[pin].cb(gpio_chan[pin].arg);
        }
    }
    if (sched_context_switch_request) {
        thread_yield();
    }
}
