semtech_loramac_t loramac;
lora_serialization_t serialization;

static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];

static void _lora_serialize_data(sensor_data_t data, lora_serialization_t* serialzation)
{
     /* Reset serialization descriptor */
    lora_serialization_reset(&serialization);
     /* Write data to serialization. Replace with your sensors measurements.
     * Keep in mind that the order in which the data is written into the
     * serialization needs to match the decoders order.
     */
    /* for all records in sensor_data_t
     * switch-case statement filling serialization depending on type of
     * vairable
     */
    /* Air temperature - HDC1000 */
    int16_t res;
    float val;

    val = (float)res/100;
    lora_serialization_write_temperature(&serialization, val);

    /* Air humidity - HDC1000 */
    val = (float)res/100;

    lora_serialization_write_humidity(&serialization, val);

    /* Visible light intensity - tsl4531x */

    lora_serialization_write_uint16(&serialization, res);

    /* Soil temperature - ds18 */
    val = (float)res/100;
    lora_serialization_write_temperature(&serialization, val);

    /* Soil moisture levels */
    lora_serialization_write_uint16(&serialization, sample);
}

#ifdef LORA_DATA_SEND_ON
static uint8_t lora_join(void)
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
    uint8_t retries = 0;
    while (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        puts("Join procedure failed. Trying again");
        retries++;

        if (retries > 2) {
            puts("Retries exceeded. LoRaWAN not joined.");
            return 1;
        }
    }
    puts("Join procedure succeeded");

    return 0;
}
#endif

static void lora_send_data(sensor_data_t *data)
{

    lora_serialization_t serialization;

    _lora_serialize_data(&data, &serialization);
    
    /* The send call blocks until done */
    semtech_loramac_send(&loramac, serialization.buffer, serialization.cursor);

    /* Wait until the send cycle has completed */
    semtech_loramac_recv(&loramac);

}
