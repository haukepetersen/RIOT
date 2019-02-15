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
#include "xtimer.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define SENDER_PRIO         (THREAD_PRIORITY_MAIN - 1)
#define WATERING_PRIO         (THREAD_PRIORITY_MAIN - 2)

static kernel_pid_t send_pid;
static char send_stack[THREAD_STACKSIZE_MAIN / 2];
static kernel_pid_t watering_pid;
static char watering_stack[THREAD_STACKSIZE_MAIN / 2];

int data_collected;

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
#define WATERING_THREAD_ON
#define HARDWARE_TEST_ON
#define LORA_DATA_SEND_ON

#define WATERING_THREAD_PERIOD              (5U)      /* watering done every 5 secs */
#define MSG_THREAD_PERIOD                   (7200U)   /* messages sent every 2 hours */
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

static void rtc_cb_watering(void *arg)
{
    (void) arg;
    msg_t msg;
    msg_send(&msg, watering_pid);
}

static void _prepare_next_alarm_watering(void)
{
    struct tm time;
    rtc_get_time(&time);

    /* set initial alarm */
    time.tm_sec += WATERING_THREAD_PERIOD;
    mktime(&time);
    rtc_set_alarm(&time, rtc_cb_watering, NULL);
}

static void *_watering(void *arg)
{
    (void)arg;
    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);

    DEBUG_PRINT("Watering thread started.\n");

    while (1) {

        DEBUG_PRINT("Watering thread.\n");
        msg_receive(&msg);

        s_and_a_update_all(data);

        /* Schedule the next wake-up alarm */
        _prepare_next_alarm_watering();

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

static void rtc_cb(void *arg)
{
    (void) arg;
    msg_t msg;
    msg_send(&msg, send_pid);
}

static void _prepare_next_alarm(void)
{
    struct tm time;
    rtc_get_time(&time);

    /* set initial alarm */
    time.tm_sec += MSG_THREAD_PERIOD;
    mktime(&time);
    rtc_set_alarm(&time, rtc_cb, NULL);
}

static void *_send(void *arg)
{
    (void)arg;
    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);

    DEBUG_PRINT("Data send thread started.\n");

    while (1) {

        DEBUG_PRINT("Data send thread.\n");

        msg_receive(&msg);

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
    /* Initial update */
    s_and_a_update_all(data);

    /* start the threads */
#ifdef WATERING_THREAD_ON
    watering_pid = thread_create(watering_stack, sizeof(watering_stack),
                               WATERING_PRIO, 0, _watering, NULL, "Watering");
#endif
    send_pid = thread_create(send_stack, sizeof(send_stack),
                               SENDER_PRIO, 0, _send, NULL, "Data collect and send");

     /* trigger the first send */
    msg_t msg_datasend;
    msg_send(&msg_datasend, send_pid);
    msg_t msg_watering;
    msg_send(&msg_watering, watering_pid);

    return 0;
}
