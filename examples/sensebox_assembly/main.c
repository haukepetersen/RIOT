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

#include "shell.h"
#include "periph/gpio.h"
#include "periph/adc.h"
#include "xtimer.h"
#include "msg.h"
#include "thread.h"
#include "fmt.h"
 #include "periph/rtc.h"
 #include "net/loramac.h"
#include "semtech_loramac.h"
#include "lora_serialization.h"

#define RES             ADC_RES_10BIT

/* Messages are sent every 20s to respect the duty cycle on each channel */
#define PERIOD              (20U)
 #define SENDER_PRIO         (THREAD_PRIORITY_MAIN - 1)
//static kernel_pid_t sender_pid;
//static char sender_stack[THREAD_STACKSIZE_MAIN / 2];
// semtech_loramac_t loramac;
//lora_serialization_t serialization;
// static uint8_t deveui[LORAMAC_DEVEUI_LEN];
//static uint8_t appeui[LORAMAC_APPEUI_LEN];
//static uint8_t appkey[LORAMAC_APPKEY_LEN];
//
//static void rtc_cb(void *arg)
//{
//    (void) arg;
//    msg_t msg;
//    msg_send(&msg, sender_pid);
//}
//
//static void _prepare_next_alarm(void)
//{
//    struct tm time;
//    rtc_get_time(&time);
//    /* set initial alarm */
//    time.tm_sec += PERIOD;
//    mktime(&time);
//    rtc_set_alarm(&time, rtc_cb, NULL);
//}
//
//static void _send_message(void)
//{
//    printf("Sending data\n");
//     /* Reset serialization descriptor */
//    lora_serialization_reset(&serialization);
//     /* Write data to serialization. Replace with your sensors measurements.
//     * Keep in mind that the order in which the data is written into the
//     * serialization needs to match the decoders order.
//     */
//    lora_serialization_write_temperature(&serialization, 25.3);
//    lora_serialization_write_uint8(&serialization, 82);
//    lora_serialization_write_humidity(&serialization, 50.2);
//    /* The send call blocks until done */
//    semtech_loramac_send(&loramac, serialization.buffer, serialization.cursor);
//    /* Wait until the send cycle has completed */
//    semtech_loramac_recv(&loramac);
//}
//
//static void *sender(void *arg)
//{
//    (void)arg;
//     msg_t msg;
//    msg_t msg_queue[8];
//    msg_init_queue(msg_queue, 8);
//     while (1) {
//        msg_receive(&msg);
//         /* Trigger the message send */
//        _send_message();
//         /* Schedule the next wake-up alarm */
//        _prepare_next_alarm();
//    }
//     /* this should never be reached */
//    return NULL;
//}
// int main(void)
//{
//    puts("LoRaWAN Class A low-power application");
//    puts("=====================================");
//    puts("Integration with TTN and openSenseMap");
//    puts("=====================================");
//     /* Convert identifiers and application key */
//    fmt_hex_bytes(deveui, DEVEUI);
//    fmt_hex_bytes(appeui, APPEUI);
//    fmt_hex_bytes(appkey, APPKEY);
//     /* Initialize the loramac stack */
//    semtech_loramac_init(&loramac);
//    semtech_loramac_set_deveui(&loramac, deveui);
//    semtech_loramac_set_appeui(&loramac, appeui);
//    semtech_loramac_set_appkey(&loramac, appkey);
//     /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
//    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);
//     /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
//     * generated device address and to get the network and application session
//     * keys.
//     */
//    puts("Starting join procedure");
//    if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
//        puts("Join procedure failed");
//        return 1;
//    }
//    puts("Join procedure succeeded");
//     /* start the sender thread */
//    sender_pid = thread_create(sender_stack, sizeof(sender_stack),
//                               SENDER_PRIO, 0, sender, NULL, "sender");
//     /* trigger the first send */
//    msg_t msg;
//    msg_send(&msg, sender_pid);
//    return 0;
//}

int main(void)
{
    puts("Welcome to RIOT!\n");
    puts("Type `help` for help, type `saul` to see all SAUL devices\n");

    /* Enables the UART 5V lines */
    gpio_init(GPIO_PIN(PB,2), GPIO_OUT);
    gpio_set(GPIO_PIN(PB, 2));

    for (unsigned i = 0; i < 5; i = i + 2) {
        if (adc_init(ADC_LINE(i)) < 0) {
            printf("ADC %u inihialization failed\n", i);
        }
        else {
            int sample = adc_sample(ADC_LINE(i), RES);
            if (sample < 0) {
                printf("ADC_LINE(%u): 10-bit resolution not applicable\n", i);
            } else {
                printf("ADC_LINE(%u): %i\n", i, sample);
            }
        }
    }
        
   /* valve 1 */ 
//    gpio_init(GPIO_PIN(PA, 7), GPIO_OUT);
//    gpio_init(GPIO_PIN(PA, 5), GPIO_OUT);
    gpio_init(GPIO_PIN(PB, 8), GPIO_OUT);
   //     .rx_pin = GPIO_PIN(PB, 9),
    while(1){
        xtimer_sleep(3);
        puts("pin clear");
//        gpio_clear(GPIO_PIN(PA, 5));
//        gpio_clear(GPIO_PIN(PA, 7));
        gpio_clear(GPIO_PIN(PB, 8));
        xtimer_sleep(3);
        for (unsigned i = 0; i < 5; i = i + 2) {
            int sample = adc_sample(ADC_LINE(i), RES);
            if (sample < 0) {
                printf("ADC_LINE(%u): 10-bit resolution not applicable\n", i);
            } else {
                printf("ADC_LINE(%u): %i\n", i, sample);
            }
        }
        xtimer_sleep(3);
        puts("pin set");
//        gpio_set(GPIO_PIN(PA, 5));
//        gpio_set(GPIO_PIN(PA, 7));
        gpio_set(GPIO_PIN(PB, 8));
        xtimer_sleep(3);
        for (unsigned i = 0; i < 5; i = i + 2) {
            int sample = adc_sample(ADC_LINE(i), RES);
            if (sample < 0) {
                printf("ADC_LINE(%u): 10-bit resolution not applicable\n", i);
            } else {
                printf("ADC_LINE(%u): %i\n", i, sample);
            }
        }
    }
    
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
