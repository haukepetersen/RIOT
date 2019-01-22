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

#include "shell.h"
#include "msg.h"
#include "thread.h"

#define SENDER_PRIO         (THREAD_PRIORITY_MAIN - 1)

static kernel_pid_t collect_and_send_pid;
static char collect_and_send_stack[THREAD_STACKSIZE_MAIN / 2];

typedef enum {
    SENSOR_DATA_T_TEMP,
    SENSOR_DATA_T_HUM,
    SENSOR_DATA_T_UINT16
} data_type_te;

typedef struct {
    uint16_t raw;
    data_type_te type;
} data_t;

/*********** Configurations ******************/
#define WATERING_ON
#define HARDWARE_TEST_ON
#define LORA_DATA_SEND_ON

#define PERIOD              (7200U)   /* messages sent every 20 mins */
#define SAFETY_LOWER        (100)
#define SAFETY_UPPER        (300)
#define WATERING_TIME       (3)
/***********************************************/

/* TODO: go through and make sure that everything is encapsulated:
 * modules only know about things they need to know about. eg the main module
 * doesn't need to know details of the hardware; neither does LoRa (it only
 * needs to know the data)
 */

/* TODO: make it so that everything that needs changing between the two setups
 * is in header files only: one for mine, one for stefan's.
 */

extern void hardware_test(void);
extern uint8_t lora_join(void);
extern void lora_send_data(data_t* data, int len);
extern int s_and_a_init_all(data_t* data); 
extern void s_and_a_update_all(data_t* data);
extern void s_and_a_hardware_test(void);

/* TODO: this means the size of data[] needs to be the same as
 * the number of sensors, manually. Make it so it's easier to change */
data_t data[7];
#define DATA_NUMOF      sizeof(data) / sizeof(data[0])

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

        s_and_a_update_all(data);

#ifdef LORA_DATA_SEND_ON
        lora_send_data(data, DATA_NUMOF);
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
    if (lora_join() > 0) {
        puts("LoRa OTA activation failed");
    }
#endif

    /* Enable the UART 5V lines */
    /* TODO: I don't actually need this for Stefan's setup. But I do for mine.
     * Move to sensebox config and raise a PR.
     */
    gpio_init(GPIO_PIN(PB, 2), GPIO_OUT);
    gpio_set(GPIO_PIN(PB, 2));

    s_and_a_init_all(data);

#ifdef HARDWARE_TEST_ON
    s_and_a_hardware_test();
#endif

    /* start the main thread */
    collect_and_send_pid = thread_create(collect_and_send_stack, sizeof(collect_and_send_stack),
                               SENDER_PRIO, 0, _collect_and_send, NULL, "Data collect and send");

     /* trigger the first send */
    msg_t msg;
    msg_send(&msg, collect_and_send_pid);

    return 0;
}
