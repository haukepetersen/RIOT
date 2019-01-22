#include <stdint.h>
#include "net/loramac.h"
#include "semtech_loramac.h"
#include "lora_serialization.h"
#include "fmt.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

typedef enum {
    SENSOR_DATA_T_TEMP,
    SENSOR_DATA_T_HUM,
    SENSOR_DATA_T_UINT16
} data_type_te;

typedef struct {
    uint16_t raw;
    data_type_te type;
} data_t;

semtech_loramac_t loramac;
lora_serialization_t serialization;

static void _lora_serialize_data(data_t* data, int data_len, lora_serialization_t* serialization)
{
    assert(data);

    lora_serialization_reset(serialization);
    DEBUG_PRINT("LoRa serialization reset.\n");

    for (int i = 0; i < data_len; i++) {

        DEBUG_PRINT("Data point %d, data type %d, raw data %d\n", i, data->type, data->raw);
        fflush(stdout);

        float cents = (float)data->raw/100;
        switch ( data->type ) {
            case SENSOR_DATA_T_TEMP:	
                lora_serialization_write_temperature(serialization, cents);
                break;

            case SENSOR_DATA_T_HUM:	
                lora_serialization_write_humidity(serialization, cents);
                break;

            case SENSOR_DATA_T_UINT16:	
                lora_serialization_write_uint16(serialization, data->raw);
                break;

            default:	
                break;
        }

        data++;
    }
}

void lora_send_data(data_t *data, int len)
{
    DEBUG_PRINT("Sending data.\n");

    lora_serialization_t serialization;

    _lora_serialize_data(data, len, &serialization);

    DEBUG_PRINT("Data serialized.\n");
    
    /* The send call blocks until done */
    semtech_loramac_send(&loramac, serialization.buffer, serialization.cursor);

    DEBUG_PRINT("Data sent.\n");

    /* Wait until the send cycle has completed */
    semtech_loramac_recv(&loramac);

    DEBUG_PRINT("Data confirmation received.\n");

}

uint8_t lora_join(void)
{
    uint8_t deveui[LORAMAC_DEVEUI_LEN];
    uint8_t appeui[LORAMAC_APPEUI_LEN];
    uint8_t appkey[LORAMAC_APPKEY_LEN];

    /* Convert identifiers and application key */
    fmt_hex_bytes(deveui, DEVEUI);
    fmt_hex_bytes(appeui, APPEUI);
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

    for(int retries = 0; retries < 3; retries++) {
        if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) == SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
            puts("Join procedure succeeded");
            return 0;
        }
    }

    puts("Join procedure failed. LoRaWAN not joined.");
    return 1;
}
