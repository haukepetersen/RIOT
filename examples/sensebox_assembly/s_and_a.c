int s_and_a_init(void){
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
}

int s_and_a_read(sensor_t sensor);
int s_and_a_water(int watering_time_secs);

static void _collect_data(lora_serialization_t *serialization, sensor_t sensor)
{

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

    sample = adc_sample(MOISTURE_SENSOR_2_PIN, RES);
    lora_serialization_write_uint16(&serialization, sample);

    sample = adc_sample(MOISTURE_SENSOR_3_PIN, RES);
    lora_serialization_write_uint16(&serialization, sample);

    gpio_clear(SENSOR_POWER_PIN);
}

void s_and_a_water(valve_t valve, int moisture_level, int watering_time){

    if (moisture_level > valve.watering_level) {
        gpio_set(valve.control_pin);
        xtimer_sleep(watering_time);
        gpio_clear(valve.control_pin);
    }

}
