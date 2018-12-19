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

#define PERIOD              (1200U)   /* messages sent every 20 mins */
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
static bool _safety_test(int sample, uint8_t valve_no)
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

static void _send_message(void)
{
    printf("Collecting data\n");

     /* Reset serialization descriptor */
    lora_serialization_reset(&serialization);

     /* Write data to serialization. Replace with your sensors measurements.
     * Keep in mind that the order in which the data is written into the
     * serialization needs to match the decoders order.
     */
    
    /* Air temperature - HDC1000 */
    int16_t res;
    float val;
    hdc1000_read_cached((const hdc1000_t *)&hdc1000_dev, &res, NULL);
    val = (float)res/100;
    lora_serialization_write_temperature(&serialization, val);

    /* Air humidity - HDC1000 */
    hdc1000_read_cached((const hdc1000_t *)&hdc1000_dev, NULL, &res);
    val = (float)res/100;
    lora_serialization_write_humidity(&serialization, val);

    /* Visible light intensity - tsl4531x */
    res = tsl4531x_simple_read(&tsl4531x_dev);
    lora_serialization_write_uint16(&serialization, res);

    /* Soil temperature - ds18 */
    if (ds18_get_temperature(&ds18_dev, &res) < 0) {
        puts("Soil temp collection failed");
        res = 0;
    }
    val = (float)res/100;
    lora_serialization_write_temperature(&serialization, val);

    /* Soil moisture levels */
    gpio_set(SENSOR_POWER_PIN);
    /* Sleep for 10s - the electrolytic effect means that it takes ~5-6s for
     * the reading to settle down
     */
    xtimer_sleep(10);
    int sample;
    sample = adc_sample(MOISTURE_SENSOR_1_PIN, RES);
    lora_serialization_write_uint16(&serialization, sample);
#ifdef VALVE_WATERING_ON
    if (_safety_test(sample, VALVE_1) && (sample > valves[VALVE_1].watering_level)) {
        gpio_set(VALVE_CONTROL_1_PIN);
        xtimer_sleep(VALVE_WATERING_TIME);
        gpio_clear(VALVE_CONTROL_1_PIN);
    }
#endif

    sample = adc_sample(MOISTURE_SENSOR_2_PIN, RES);
    lora_serialization_write_uint16(&serialization, sample);
#ifdef VALVE_WATERING_ON
    if (_safety_test(sample, VALVE_2) && (sample > valves[VALVE_2].watering_level)) {
        gpio_set(VALVE_CONTROL_2_PIN);
        xtimer_sleep(VALVE_WATERING_TIME);
        gpio_clear(VALVE_CONTROL_2_PIN);
    }
#endif

    sample = adc_sample(MOISTURE_SENSOR_3_PIN, RES);
    lora_serialization_write_uint16(&serialization, sample);

    gpio_clear(SENSOR_POWER_PIN);

#ifdef LORA_DATA_SEND_ON
    /* The send call blocks until done */
    semtech_loramac_send(&loramac, serialization.buffer, serialization.cursor);

    /* Wait until the send cycle has completed */
    semtech_loramac_recv(&loramac);
#endif

}

#ifdef HARDWARE_TEST_ON
static void _hardware_test(void)
{
    /* Test the valves */
    puts("Testing the valves. Valves should click on and off three times each.");
    gpio_set(SENSOR_POWER_PIN);
    xtimer_sleep(1);
    for (int i = 0; i < 3; i++) {
        gpio_set(VALVE_CONTROL_1_PIN);
        xtimer_sleep(1);
        gpio_clear(VALVE_CONTROL_1_PIN);
        xtimer_sleep(1);
    }
    for (int i = 0; i < 3; i++) {
        gpio_set(VALVE_CONTROL_2_PIN);
        xtimer_sleep(1);
        gpio_clear(VALVE_CONTROL_2_PIN);
        xtimer_sleep(1);
    }

    /* Test the soil temp, air temp/humidity, and light sensors */
    puts("Testing the soil temp, air temp/humidity, and light sensors. Check the readings are sensible.");
    int16_t res;
    float val;
    hdc1000_read_cached((const hdc1000_t *)&hdc1000_dev, &res, NULL);
    val = (float)res/100;
    /* TODO: this won't print val, for some reason. Get it printing... maybe it
     * needs a double format specifier?*/
    (void)val;
    printf("Air temperature is %d.\n", res);

    hdc1000_read_cached((const hdc1000_t *)&hdc1000_dev, NULL, &res);
    val = (float)res/100;
    printf("Air humidity is %d.\n", res);

    /* Visible light intensity - tsl4531x */
    res = tsl4531x_simple_read(&tsl4531x_dev);
    printf("Light intensity is %d\n", res);

    /* Soil temperature - ds18 */
    gpio_set(SENSOR_POWER_PIN);
    xtimer_sleep(1);
    if (ds18_get_temperature(&ds18_dev, &res) < 0) {
        puts("Soil temp collection failed");
        res = 0;
    }
    val = (float)res/100;
    printf("Soil temperature is %d.\n", res);

    /* Test the power line for the moisture sensors */
    puts("Testing moisture sensor power. Light on control board should be blinking on and off.");
    for (int i = 0; i < 3; i++) {
        gpio_set(SENSOR_POWER_PIN);
        xtimer_sleep(1);
        gpio_clear(SENSOR_POWER_PIN);
        xtimer_sleep(1);
    }

    /* Test the moisture sensors */
    puts("Testing moisture sensors. Check the readings are sensible.");
    gpio_set(SENSOR_POWER_PIN);
    /* Sleep for 10s - the electrolytic effect means that it takes ~5-6s for
     * the reading to settle down
     */
    xtimer_sleep(10);
    int sample = adc_sample(MOISTURE_SENSOR_1_PIN, RES);
    printf("Plant 1 moisture level is %d.\n", sample);
    sample = adc_sample(MOISTURE_SENSOR_2_PIN, RES);
    printf("Plant 2 moisture level is %d.\n", sample);
    sample = adc_sample(MOISTURE_SENSOR_3_PIN, RES);
    printf("Plant 3 moisture level is %d.\n", sample);
    gpio_clear(SENSOR_POWER_PIN);


}
#endif

static void *sender(void *arg)
{
    (void)arg;
     msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);
     while (1) {
        msg_receive(&msg);
         /* Trigger the message send */
        _send_message();
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
     /* Convert identifiers and application key */
    fmt_hex_bytes(deveui, DEVEUI); fmt_hex_bytes(appeui, APPEUI);
    fmt_hex_bytes(appkey, APPKEY);

#ifdef LORA_DATA_SEND_ON
     /* Initialize the loramac stack */
    semtech_loramac_init(&loramac);
    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);
     /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);
     /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
     * generated device address and to get the network and application session
     * keys.
     */
    puts("Starting join procedure");
    if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        puts("Join procedure failed");
        return 1;
    }
    puts("Join procedure succeeded");
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
    _hardware_test();
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
