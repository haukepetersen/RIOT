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
#include "periph/adc.h"
#include "periph/rtc.h"

#include "shell.h"
#include "xtimer.h"
#include "msg.h"
#include "thread.h"
#include "fmt.h"
#include "net/loramac.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

#include "semtech_loramac.h"
#include "lora_serialization.h"

#include "hdc1000.h"
#include "hdc1000_params.h"
#include "tsl4531x.h"
#include "tsl4531x_params.h"
#include "ds18.h"
#include "ds18_params.h"


#define SENDER_PRIO         (THREAD_PRIORITY_MAIN - 1)

static kernel_pid_t collect_and_send_pid;
static char collect_and_send_stack[THREAD_STACKSIZE_MAIN / 2];

/*********** Configurations ******************/
#define VALVE_WATERING_ON
//#define HARDWARE_TEST_ON
#define LORA_DATA_SEND_ON

#define PERIOD              (7200U)   /* messages sent every 20 mins */
#define SAFETY_LOWER        (100)
#define SAFETY_UPPER        (300)
#define WATERING_TIME       (3)
/***********************************************/

extern void hardware_test(void);
extern void lora_join(void);
extern int s_and_a_init(void); 
extern void s_and_a_update(sensor_t sensors);
extern void s_and_a_water(valve_t valves);

static void rtc_cb(void *arg)
{
    (void) arg;
    msg_t msg;
    msg_send(&msg, collect_and_send_pid);
}

static void _prepare_next_alarm(void)
{
    struct tm time;
    rtc_get_time(&time);

    /* set initial alarm */
    time.tm_sec += PERIOD;
    mktime(&time);
    rtc_set_alarm(&time, rtc_cb, NULL);
}

static void *_collect_and_send(void *arg)
{
    (void)arg;
    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);

    while (1) {

        msg_receive(&msg);

        /* Collect data from sensors */
        /* for all sensors, s_and_a_read() */
        for (int i = 0; i < SENSOR_NUMOF; i++) {
            s_and_a_update(sensors[i]);
        }

#ifdef WATERING_ON
        for (int i = 0; i < VALVE_NUMOF; i++) {
            s_and_a_water(valves[i]);
        }
#endif

#ifdef LORA_DATA_SEND_ON
        lora_send_data(&sensors);
#endif

        /* Schedule the next wake-up alarm */
        _prepare_next_alarm();

        /* TODO: Try this for memory usage, replacing "rtt" with "rtc"
         * and removing all the msg stuff
         *
         * rtt_set_alarm(sleepDuration + startTime, rttCallback, NULL);
         * thread_sleep();
         *
         * then wake with a callback:
         *void rttCallback(void *arg) {
             thread_wakeup(MainPid);
             }
         *
         * because it involves less lines of code
         */

    }
    /* this should never be reached */
    return NULL;
}

int main(void)
{
    puts("LoRaWAN Class A low-power application");
    puts("=====================================");
    puts("Integration with TTN and openSenseMap");
    puts("=====================================");

#ifdef LORA_DATA_SEND_ON
    if (lora_join() < 0) {
        puts("LoRa OTA activation failed");
    }
#endif

    /* Enable the UART 5V lines */
    /* TODO: I don't actually need this for Stefan's setup. But I do for mine.
     * Move to sensebox config and raise a PR.
     */
    gpio_init(GPIO_PIN(PB, 2), GPIO_OUT);
    gpio_set(GPIO_PIN(PB, 2));

    s_and_a_init_all();

#ifdef HARDWARE_TEST_ON
    hardware_test();
#endif

    /* start the main thread */
    collect_and_send_pid = thread_create(collect_and_send_stack, sizeof(collect_and_send_stack),
                               SENDER_PRIO, 0, _collect_and_send, NULL, "Data collect and send");

     /* trigger the first send */
    msg_t msg;
    msg_send(&msg, collect_and_send_pid);

    return 0;
}
