/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "periph/adc.h"

#ifndef LINE
#define LINE        (0)
#endif

#define BITS        (0xf)

adc_res_t try[] = { ADC_RES_16BIT, ADC_RES_14BIT, ADC_RES_12BIT,
                    ADC_RES_10BIT, ADC_RES_8BIT, ADC_RES_6BIT };

int main(void)
{
    puts("ADC entropy test!");

    /* init ADC */
    if (adc_init(ADC_LINE(LINE)) != 0) {
        printf("unable to initialize ADC_LINE(%i)\n", LINE);
        return 1;
    }

    /* get highest possible ADC resolution */
    adc_res_t res;
    int err = -1;
    for (int i = 0; (i < 6) && (err < 0); i++) {
        res = try[i];
        err = adc_sample(ADC_LINE(LINE), res);
        if (err >= 0) {
            printf("got res %i\n", i);
        }
    }

    for (int i = 0; i < 4000000; i++) {
        int val = 0;
        for (int iter = 0; iter < 4; iter ++) {
            int sample = adc_sample(ADC_LINE(LINE), res);
            val |= (sample & 0x3) << (iter * 2);
        }
        printf("%i\n", val);
    }

    return 0;
}
