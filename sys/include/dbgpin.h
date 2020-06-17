/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_dbgpin Direct pin control for debugging/profiling
 * @ingroup     sys
 *
 * @warning     This module does not verify the given pin number, so make sure
 *              the pin numbers you use are actually configured!
 *
 * @{
 * @file
 * @brief       GPIO wrapper for debugging/profiling purposes
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#include "kernel_defines.h"
#include "periph/gpio.h"
#include "dbgpin_params.h"

static inline void dbgpin_set(unsigned pin)
{
    gpio_set(dbgpin_params[pin]);
}

static inline void dbgpin_clr(unsigned pin)
{
    gpio_clear(dbgpin_params[pin]);
}

static inline void dbgpin_tgl(unsigned pin)
{
    gpio_toggle(dbgpin_params[pin]);
}

static inline void dbgpin_sig(unsigned pin)
{
    dbgpin_tgl(pin);
    dbgpin_tgl(pin);
}

static inline unsigned dbgpin_cnt(void)
{
    return ARRAY_SIZE(dbgpin_params);
}

static inline void dbgpin_init(void)
{
    for (unsigned i = 0; i < dbgpin_cnt(); i++) {
        gpio_init(dbgpin_params[i], GPIO_OUT);
        gpio_clear(dbgpin_params[i]);
    }
}
