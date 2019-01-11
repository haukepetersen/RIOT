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
 * @brief       Tests for correct operation of hardware.
 *
 * @author      Daniel Petry <daniel.petry@fu-berlin.de>
 *
 * @}
 */
/*TODO: sort out doxygen structure/categories/whatever */

#ifdef HARDWARE_TEST_ON
void hardware_test(void)
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
