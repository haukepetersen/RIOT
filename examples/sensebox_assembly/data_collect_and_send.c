
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
#ifdef LORA_DATA_SEND_ON
static uint8_t _lora_join(void)
{

    /* Convert identifiers and application key */
    fmt_hex_bytes(deveui, DEVEUI); fmt_hex_bytes(appeui, APPEUI);
    fmt_hex_bytes(appkey, APPKEY);

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

    return 0;

}
#endif
static void _collect_data_and_control_valves(lora_serialization_t *serialization)
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
}
static void _send_message(void)
{
    printf("Collecting data\n");

     /* Reset serialization descriptor */
    lora_serialization_reset(&serialization);

     /* Write data to serialization. Replace with your sensors measurements.
     * Keep in mind that the order in which the data is written into the
     * serialization needs to match the decoders order.
     */
    
    collect_data_and_control_valves(&serialization);

#ifdef LORA_DATA_SEND_ON
    /* The send call blocks until done */
    semtech_loramac_send(&loramac, serialization.buffer, serialization.cursor);

    /* Wait until the send cycle has completed */
    semtech_loramac_recv(&loramac);
#endif

}


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
