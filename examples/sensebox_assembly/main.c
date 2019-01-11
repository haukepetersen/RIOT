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


static void rtc_cb(void *arg)
{
    (void) arg;
    msg_t msg;
    msg_send(&msg, sender_pid);
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
#ifdef VALVE_WATERING_ON
bool _safety_test(int sample, uint8_t valve_no)
{
    if (sample < (valves[valve_no].watering_level - SAFETY_LOWER)){
        valves[valve_no].low_disable = true;
    }

    if (sample > (valves[valve_no].watering_level)) {
        valves[valve_no].low_disable = false;
    }

    if (sample > (valves[valve_no].watering_level + SAFETY_UPPER)){
        valves[valve_no].high_disable = true;
    }

    DEBUG_PRINT("valve - %4d, watering level - %4d, sample value - %4d,",
            valve_no, valves[valve_no].watering_level, sample);
    DEBUG_PRINT("low disable - %1d, high disable - %1d\n",
            valves[valve_no].low_disable, valves[valve_no].high_disable);

    if (valves[valve_no].low_disable || valves[valve_no].high_disable) {
        return false;
    }

    return true;
}
#endif

void _configure_application(void){

#ifdef VALVE_WATERING_ON
    /* Set the watering level */
    valves[VALVE_1].watering_level = VALVE_1_WATERING_LEVEL_INIT;
    printf("Plant 1 watering level is %d.\n", valves[VALVE_1].watering_level);
    valves[VALVE_2].watering_level = VALVE_2_WATERING_LEVEL_INIT;
    printf("Plant 2 watering level is %d.\n", valves[VALVE_2].watering_level);
    gpio_clear(SENSOR_POWER_PIN);
#endif
}

static void *_sender(void *arg)
{
    (void)arg;
     msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);
     while (1) {
        msg_receive(&msg);

        /* TODO: another way to wake a thread up periodically? there's no msg
         * being sent.
         * use xtimer_periodic_wakeup().
         */

     /* Write data to serialization. Replace with your sensors measurements.
     * Keep in mind that the order in which the data is written into the
     * serialization needs to match the decoders order.
     */

        /* Collect data from sensors */
        /* for all sensors, s_and_a_read() */

        /* If _safety_test(), s_and_a_water(valve_t, time) */
         _safety_test(moisture_level, VALVE_1)
#ifdef LORA_DATA_SEND_ON
        lora_send_data(&serialization);

         /* Schedule the next wake-up alarm */
        _prepare_next_alarm();
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
    while(lora_join());
#endif

    if (s_and_a_init(void) < 0){
        puts("Sensor and actuator initialisation failed");
    }

#ifdef HARDWARE_TEST_ON
    hardware_test();
#endif


    /* start the main thread */
    sender_pid = thread_create(sender_stack, sizeof(sender_stack),
                               SENDER_PRIO, 0, _sender, NULL, "sender");

     /* trigger the first send */
    msg_t msg;
    msg_send(&msg, sender_pid);

    return 0;
}
