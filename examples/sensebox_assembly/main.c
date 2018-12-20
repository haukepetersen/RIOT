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

typedef struct {
    uint16_t watering_level;
    bool low_disable;
    bool high_disable;
} valve_t;

#define RES             ADC_RES_10BIT
// Valves
////    gpio_init(GPIO_PIN(PA, 7), GPIO_OUT);
////    gpio_init(GPIO_PIN(PA, 5), GPIO_OUT);
////        gpio_clear(GPIO_PIN(PA, 5));
////        gpio_clear(GPIO_PIN(PA, 7));

#define SENDER_PRIO         (THREAD_PRIORITY_MAIN - 1)

static kernel_pid_t sender_pid;
static char sender_stack[THREAD_STACKSIZE_MAIN / 2];
semtech_loramac_t loramac;
lora_serialization_t serialization;

static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];


/*********** Configurations ******************/
#define VALVE_WATERING_ON
//#define HARDWARE_TEST_ON
#define LORA_DATA_SEND_ON

#define PERIOD              (7200U)   /* messages sent every 20 mins */
#define SAFETY_LOWER        (100)
#define SAFETY_UPPER        (300)

#define MOISTURE_SENSOR_1_PIN      ADC_LINE(0)
#define MOISTURE_SENSOR_2_PIN      ADC_LINE(2)
#define MOISTURE_SENSOR_3_PIN      ADC_LINE(5)

#define VALVE_CONTROL_1_PIN        GPIO_PIN(PA, 5) 
#define VALVE_CONTROL_2_PIN        GPIO_PIN(PA, 7) 
#define VALVE_WATERING_TIME        (3)                /* seconds */
#define VALVE_WATERING_TYPE        WATERING_INDIVIDUAL
#define VALVE_NUMBER               (2)
#define VALVE_1                    (0)
#define VALVE_2                    (1)
#define VALVE_1_WATERING_LEVEL_INIT  (700)
#define VALVE_2_WATERING_LEVEL_INIT  (700)

#define SENSOR_POWER_PIN           GPIO_PIN(PB, 8)

/* TODO: try this from here, rather than in sensebox/include/periph_conf.h */
//#define ADC_0_REF_DEFAULT                  ADC_REFCTRL_REFSEL_AREFA

/*TODO: try this from here, rather than in ds18/include/ds18_params.h */
//#define DS18_PARAM_PIN             (GPIO_PIN(PB, 9))
//#define DS18_PARAM_PULL            (GPIO_OD_PU)

/*TODO: the sensor switching is different on the two setups. On mine, it
 * switches the valves too but not soil temp sensor, and vice versa for 
 * Stefan's. Could this be handled in config too?
 */

/* light (tsl4531x) and temp/humidity (hdc1000) drivers currently need:
 *
 * - USEMODULE += [name] in makefile
 * - a device descriptor declared here
 * - initialization by calling the required device_init function
 * - collection of data by calling the relevant functions
 * - sending data by calling lora_serialization_write
 * - handling of data in opensensemap
 *
 *   these are the same between the two setups so will remain the same in the
 *   code for the time being.
 *
 *   TODO: make this configurable too
 */

extern void hardware_test(void);

/* Sensor device descriptors */
static hdc1000_t hdc1000_dev;
static tsl4531x_t tsl4531x_dev;
static ds18_t ds18_dev;

valve_t valves[VALVE_NUMBER];



int main(void)
{
    puts("LoRaWAN Class A low-power application");
    puts("=====================================");
    puts("Integration with TTN and openSenseMap");
    puts("=====================================");

#ifdef LORA_DATA_SEND_ON
    while(_lora_join());
#endif

    /* Enable the UART 5V lines */
    /* TODO: I don't actually need this for Stefan's setup. But a PR should be
     * raised
     */
    gpio_init(GPIO_PIN(PB, 2), GPIO_OUT);
    gpio_set(GPIO_PIN(PB, 2));

    /* Initialise the sensors */
    hdc1000_init(&hdc1000_dev, &hdc1000_params[0]);
    tsl4531x_init(&tsl4531x_dev, &tsl4531x_params[0]);
    ds18_init(&ds18_dev, &ds18_params[0]);

    /* Initialise the ADCs */
    if (adc_init(MOISTURE_SENSOR_1_PIN) < 0) {
        printf("Moisture sensor 1 ADC initialization failed\n");
    }
    if (adc_init(MOISTURE_SENSOR_2_PIN) < 0) {
        printf("Moisture sensor 2 ADC initialization failed\n");
    }
    if (adc_init(MOISTURE_SENSOR_3_PIN) < 0) {
        printf("Moisture sensor 3 ADC initialization failed\n");
    }

    /* Initialise the hygrometer power control line */
    gpio_init(SENSOR_POWER_PIN, GPIO_OUT);

    /* Initialise the valve control lines */
    gpio_init(VALVE_CONTROL_1_PIN, GPIO_OUT);
    gpio_init(VALVE_CONTROL_2_PIN, GPIO_OUT);

#ifdef HARDWARE_TEST_ON
    hardware_test();
#endif

#ifdef VALVE_WATERING_ON
    /* Set the watering level */
    gpio_set(SENSOR_POWER_PIN);
    /* Sleep for 10s - the electrolytic effect means that it takes ~5-6s for
     * the reading to settle down
     */
    xtimer_sleep(10);
    //valves[VALVE_1].watering_level = adc_sample(MOISTURE_SENSOR_1_PIN, RES);
    valves[VALVE_1].watering_level = VALVE_1_WATERING_LEVEL_INIT;
    printf("Plant 1 watering level is %d.\n", valves[VALVE_1].watering_level);
    //valves[VALVE_2].watering_level = adc_sample(MOISTURE_SENSOR_2_PIN, RES);
    valves[VALVE_2].watering_level = VALVE_2_WATERING_LEVEL_INIT;
    printf("Plant 2 watering level is %d.\n", valves[VALVE_2].watering_level);
    gpio_clear(SENSOR_POWER_PIN);
#endif

    /* start the sender thread */
    sender_pid = thread_create(sender_stack, sizeof(sender_stack),
                               SENDER_PRIO, 0, sender, NULL, "sender");

     /* trigger the first send */
    msg_t msg;
    msg_send(&msg, sender_pid);
    return 0;
}

//int main(void)
//{
//    puts("Welcome to RIOT!\n");
//    puts("Type `help` for help, type `saul` to see all SAUL devices\n");
//
//    /* Enables the UART 5V lines */
//    gpio_init(GPIO_PIN(PB,2), GPIO_OUT);
//    gpio_set(GPIO_PIN(PB, 2));
//
//    for (unsigned i = 0; i < 5; i = i + 2) {
//        if (adc_init(ADC_LINE(i)) < 0) {
//            printf("ADC %u inihialization failed\n", i);
//        }
//        else {
//            int sample = adc_sample(ADC_LINE(i), RES);
//            if (sample < 0) {
//                printf("ADC_LINE(%u): 10-bit resolution not applicable\n", i);
//            } else {
//                printf("ADC_LINE(%u): %i\n", i, sample);
//            }
//        }
//    }
//        
//   /* valve 1 */ 
////    gpio_init(GPIO_PIN(PA, 7), GPIO_OUT);
////    gpio_init(GPIO_PIN(PA, 5), GPIO_OUT);
//    gpio_init(GPIO_PIN(PB, 8), GPIO_OUT);
//   //     .rx_pin = GPIO_PIN(PB, 9),
//    while(1){
//        xtimer_sleep(3);
//        puts("pin clear");
////        gpio_clear(GPIO_PIN(PA, 5));
////        gpio_clear(GPIO_PIN(PA, 7));
//        gpio_clear(GPIO_PIN(PB, 8));
//        xtimer_sleep(3);
//        for (unsigned i = 0; i < 5; i = i + 2) {
//            int sample = adc_sample(ADC_LINE(i), RES);
//            if (sample < 0) {
//                printf("ADC_LINE(%u): 10-bit resolution not applicable\n", i);
//            } else {
//                printf("ADC_LINE(%u): %i\n", i, sample);
//            }
//        }
//        xtimer_sleep(3);
//        puts("pin set");
////        gpio_set(GPIO_PIN(PA, 5));
////        gpio_set(GPIO_PIN(PA, 7));
//        gpio_set(GPIO_PIN(PB, 8));
//        xtimer_sleep(3);
//        for (unsigned i = 0; i < 5; i = i + 2) {
//            int sample = adc_sample(ADC_LINE(i), RES);
//            if (sample < 0) {
//                printf("ADC_LINE(%u): 10-bit resolution not applicable\n", i);
//            } else {
//                printf("ADC_LINE(%u): %i\n", i, sample);
//            }
//        }
//    }
//    
//    char line_buf[SHELL_DEFAULT_BUFSIZE];
//    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
//
//    return 0;
//}
