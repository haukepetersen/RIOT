/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     driver_ws2812b
 * @{
 *
 * @file
 * @brief       Implementation of the WS2812B driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdint.h>

#include "assert.h"
#include "ws2812b.h"
#include "periph_conf.h"

static inline void on(void)
{
    PORT->Group[0].OUTSET.reg = 0x80;
}

static inline void off(void)
{
    PORT->Group[0].OUTCLR.reg = 0x80;
}

int ws2812b_init(ws2812b_t *dev, const ws2812b_params_t *params)
{
    assert(dev && params);

    dev->pin = params->pin;
    gpio_init(dev->pin, GPIO_OUT);
    gpio_clear(dev->pin);

    return WS2812B_OK;
}

void ws2812b_set(const ws2812b_t *dev, const color_rgb_t *values)
{
    assert(dev & values);

    uint8_t tmp[3];

    /* convert values */
    tmp[0] = values[0].g;
    tmp[1] = values[0].r;
    tmp[2] = values[0].b;

    unsigned istate = irq_disable();

    /* write values */
    for (unsigned i = 0; i < 3; i++) {
        for (unsigned bit = 0; bit < 8; bit++) {

            on();
            __asm("nop");
            __asm("nop");
            __asm("nop");
            __asm("nop");
            __asm("nop");
            __asm("nop");
            __asm("nop");
            __asm("nop");
            __asm("nop");
            __asm("nop");

            if (tmp[i] & 0x80) {
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                off();
            }
            else {
                off();
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
                __asm("nop");
            }

            __asm("nop");
            __asm("nop");
            __asm("nop");
            tmp[i] <<= 1;
        }
    }
    /* do a last toggle before the break */
    // gpio_set(dev->pin);
    // gpio_clear(dev->pin);

    irq_restore(istate);
}
