/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for WS2812B RGB LEDs
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "xtimer.h"
#include "ws2812b.h"
#include "ws2812b_params.h"

int main(void)
{
    ws2812b_t dev;
    color_rgb_t val = { 0x00, 0x00, 0x00 };

    puts("WS2812B RGB LED test application\n");
    printf("Initializing WS2812B RGB LED(s)...   ");
    int res = ws2812b_init(&dev, ws2812b_params);
    if (res == WS2812B_OK) {
        puts("[ok]\n");
    }
    else {
        puts("[failed]");
        return 1;
    }

    puts("setting color now\n");
    while (1) {
        val.r = 0xff;
        ws2812b_set(&dev, &val);
        val.r = 0x00;
        xtimer_sleep(1);

        val.g = 0xff;
        ws2812b_set(&dev, &val);
        val.g = 0x00;
        xtimer_sleep(1);

        val.b = 0xff;
        ws2812b_set(&dev, &val);
        val.b = 0x00;
        xtimer_sleep(1);

    }
    return 0;
}
