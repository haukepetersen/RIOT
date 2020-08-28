/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the VL53L0X distance sensor driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "xtimer.h"
#include "vl53l0x.h"
#include "vl53l0x_params.h"

int main(void)
{
    int res;
    vl53l0x_t dev;

    puts("VL53L0X time-of-flight distance sensor test application\n");

    /* initialize the device */
    res = vl53l0x_init(&dev, &vl53l0x_params[0]);
    if (res == VL53L0X_OK) {
        puts("success: device intialized\n");
    }
    else {
        printf("error: device initialization failed (%i)\n", res);
        return 1;
    }

    return 0;
}
