/*
 * Copyright (C) 2017 Freie Universität Berlin
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
 * @brief       Example for demonstrating SAUL and the SAUL registry
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "periph/gpio.h"
#include "periph/rtc.h"

#include "assert.h"
#include "shell.h"
#include "msg.h"
#include "thread.h"
#include "xtimer.h"
#include "mutex.h"

#include "app.h"

#define ENABLE_DEBUG            (1)
#include "debug.h"

#ifndef HARDWARE_TEST
#define HARDWARE_TEST           (1)
#endif
#ifndef ENALBE_LORA
#define ENABLE_LORA             (0)
#endif

#if ENALBE_LORA
#define SAMPLING_ITVL           (300U)      /* every 5min */
#else
#define SAMPLING_ITVL           (1U)
#endif

#define SENSOR_NUMOF            (7U)

int main(void)
{
    static data_t data[SENSOR_NUMOF];

    puts("LoRaWAN Class A low-power application");
    puts("=====================================");
    puts("Integration with TTN and openSenseMap");
    puts("=====================================");
#if ENALBE_LORA
    lora_join();
#endif

    /* Enable the UART 5V lines */
    /* TODO: I don't actually need this for Stefan's setup. But I do for mine.
     * Move to sensebox config and raise a PR.
     */
    gpio_init(GPIO_PIN(PB, 2), GPIO_OUT);
    gpio_set(GPIO_PIN(PB, 2));

    /* initialize sensors */
    s_and_a_init_all(data);
#if HARDWARE_TEST
    s_and_a_hardware_test();
#endif

    while (1) {
        /* read sensor data */
        DEBUG("[main] collecting new sensor data\n");
        s_and_a_update_all(data);
#if ENALBE_LORA
        /* publish the data to OpenSenseMap */
        DEBUG("[main] publishing data now\n");
        lora_send_data(data, SENSOR_NUMOF);
#endif
        /* wait until next event */
        DEBUG("[main] waiting for next event\n");
        xtimer_sleep(SAMPLING_ITVL);
    }

    return 0;
}
