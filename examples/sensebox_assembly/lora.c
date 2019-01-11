
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

static void _lora_serialize_data(sensor_data_t data, lora_serialization_t* serialzation)
{
     /* Reset serialization descriptor */
    lora_serialization_reset(&serialization);
    /* for all records in sensor_data_t
     * switch-case statement filling serialization depending on type of
     * vairable
     */
}


static void lora_send_data(sensor_data_t *data)
{

    lora_serialization_t serialization;

    _lora_serialize_data(&data, &serialization);
    
    /* The send call blocks until done */
    semtech_loramac_send(&loramac, serialization.buffer, serialization.cursor);

    /* Wait until the send cycle has completed */
    semtech_loramac_recv(&loramac);

}


