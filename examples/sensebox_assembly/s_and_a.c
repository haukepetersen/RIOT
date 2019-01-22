#include "periph/gpio.h"
#include "periph/adc.h"
#include "xtimer.h"
#include "hdc1000.h"
#include "hdc1000_params.h"
#include "tsl4531x.h"
#include "tsl4531x_params.h"
#include "ds18.h"
#include "ds18_params.h"
#include "debug.h"

typedef enum {
    SENSOR_DATA_T_TEMP,
    SENSOR_DATA_T_HUM,
    SENSOR_DATA_T_UINT16
} data_type_te;

/*TODO: make this defined in only one place in the code */
typedef struct {
    uint16_t raw;
    data_type_te type;
} data_t;

/*TODO: this depends on the hardware. any way to remove it from here? */
#define RES                 ADC_RES_10BIT
#define GLOBAL_POWER_PIN    GPIO_PIN(PB, 8)
#define GLOBAL_STARTUP_TIME (10)

#define ENABLE_DEBUG        (0)

/* TODO: put into hygrometer.h */
typedef struct {
    adc_t read_pin;
} hygrometer_params_t;

typedef struct {
    const hygrometer_params_t* params;
} hygrometer_t;

/* TODO: put into hygrometer_params.h */
/* TODO: implement one 10s startup time for all sensors */
#define HYGROMETER_1_READ_PIN      ADC_LINE(0)
#define HYGROMETER_2_READ_PIN      ADC_LINE(2)
#define HYGROMETER_3_READ_PIN      ADC_LINE(5)

static const hygrometer_params_t hygrometer_params[] = 
{
    {
        .read_pin = HYGROMETER_1_READ_PIN
    },
    {
        .read_pin = HYGROMETER_2_READ_PIN
    },
    {
        .read_pin = HYGROMETER_3_READ_PIN
    }
};

/* TODO: put into s_and_a.h */
typedef enum {
    SENSOR_T_HDC1000_TEMP,
    SENSOR_T_HDC1000_HUM,
    SENSOR_T_TSL4531X,
    SENSOR_T_DS18,
    SENSOR_T_HYGROMETER
} sensor_type_te;

typedef struct {
    /* TODO: deduplicate raw_data and data_type. maybe a pointer to a data_t?
     * Or maybe main.c just needs to have knowledge of sensor_t, or something.
     * Remember the encapsulation
     * within s_and_a.c, though. Maybe I need an s_and_a_read() function or the
     * like. */
    uint8_t name[12];
    int16_t raw_data;
    data_type_te data_type;
    sensor_type_te sensor_type;
    uint8_t config_no;
    void* dev;
} sensor_t;

typedef struct {
    uint8_t watering_time;
    uint16_t watering_level;
    uint16_t safety_margin_lower;
    uint16_t safety_margin_upper;
    bool low_disable;
    bool high_disable;
    gpio_t control_pin;
    sensor_t* control_sensor;
} valve_t;

/* TODO: put into s_and_a_params.h */
#define VALVE_1_CONTROL_PIN        GPIO_PIN(PA, 5)
#define VALVE_1_WATERING_LEVEL     (700)
#define VALVE_1_WATERING_TIME      (3)                /* seconds */
#define VALVE_1_SAFETY_LOWER       (100)
#define VALVE_1_SAFETY_UPPER       (200)

#define VALVE_2_CONTROL_PIN        GPIO_PIN(PA, 7)
#define VALVE_2_WATERING_LEVEL     (700)
#define VALVE_2_WATERING_TIME      (3)                /* seconds */
#define VALVE_2_SAFETY_LOWER       (100)
#define VALVE_2_SAFETY_UPPER       (200)

/* TODO: separate out a sensor_conf and a valve_conf, following
 * the style of periph_conf.h. Change the init function accordingly,
 * so that periph is linked to the dev pointer during init
 */

static hdc1000_t hdc1000_dev;
static tsl4531x_t tsl4531x_dev;
static ds18_t ds18_dev;
static hygrometer_t hygrometer_1_dev;
static hygrometer_t hygrometer_2_dev;
static hygrometer_t hygrometer_3_dev;

static sensor_t sensors[] =
{
    {
        .name = "hdc1000 temp",
        .raw_data = 0,
        .data_type = SENSOR_DATA_T_TEMP,
        .sensor_type = SENSOR_T_HDC1000_TEMP,
        .config_no = 0,
        .dev = &hdc1000_dev
    },
    {
        .name = "hdc1000 hum",
        .raw_data = 0,
        .data_type = SENSOR_DATA_T_HUM,
        .sensor_type = SENSOR_T_HDC1000_HUM,
        .config_no = 0,
        .dev = &hdc1000_dev
    },
    {
        .name = "tsl4531x",
        .raw_data = 0,
        .data_type = SENSOR_DATA_T_UINT16,
        .sensor_type = SENSOR_T_TSL4531X,
        .config_no = 0,
        .dev = &tsl4531x_dev
    },
    {
        .name = "ds18",
        .raw_data = 0,
        .data_type = SENSOR_DATA_T_TEMP,
        .sensor_type = SENSOR_T_DS18,
        .config_no = 0,
        .dev = &ds18_dev
    },
    {
        .name = "hygrometer 1",
        .raw_data = 0,
        .data_type = SENSOR_DATA_T_UINT16,
        .sensor_type = SENSOR_T_HYGROMETER,
        .config_no = 0,
        .dev = &hygrometer_1_dev
    },
    {
        .name = "hygrometer 2",
        .raw_data = 0,
        .data_type = SENSOR_DATA_T_UINT16,
        .sensor_type = SENSOR_T_HYGROMETER,
        .config_no = 1,
        .dev = &hygrometer_2_dev
    },
    {
        .name = "hygrometer 3",
        .raw_data = 0,
        .data_type = SENSOR_DATA_T_UINT16,
        .sensor_type = SENSOR_T_HYGROMETER,
        .config_no = 2,
        .dev = &hygrometer_3_dev
    }
};

#define SENSOR_NUMOF           sizeof(sensors) / sizeof(sensors[0])

static valve_t valves[] = 
{
    {
        .watering_time =  VALVE_1_WATERING_TIME,
        .watering_level = VALVE_1_WATERING_LEVEL,
        .safety_margin_lower = VALVE_1_SAFETY_LOWER,
        .safety_margin_upper = VALVE_1_SAFETY_UPPER,
        .low_disable = false,
        .high_disable = false,
        .control_pin = VALVE_1_CONTROL_PIN,
        .control_sensor = &sensors[0]
    },
    {
        .watering_time =  VALVE_2_WATERING_TIME,
        .watering_level = VALVE_2_WATERING_LEVEL,
        .safety_margin_lower = VALVE_2_SAFETY_LOWER,
        .safety_margin_upper = VALVE_2_SAFETY_UPPER,
        .low_disable = false,
        .high_disable = false,
        .control_pin = VALVE_2_CONTROL_PIN,
        .control_sensor = &sensors[1]
    }
};

#define VALVE_NUMOF           sizeof(valves) / sizeof(valves[0])

/* TODO: try this from here, rather than in sensebox/include/periph_conf.h */
//#define ADC_0_REF_DEFAULT                  ADC_REFCTRL_REFSEL_AREFA

/*TODO: try this from here, rather than in ds18/include/ds18_params.h */
//#define DS18_PARAM_PIN             (GPIO_PIN(PB, 9))
//#define DS18_PARAM_PULL            (GPIO_OD_PU)

/*TODO: the sensor switching is different on the two setups. On mine, it
 * switches the valves too but not soil temp sensor, and vice versa for 
 * Stefan's. Could this be handled in config too?
 */

/* TODO: once this is made truly generic, i.e. no code needs to be changed
 * for adding or removing sensors, then these descriptors will be able to be
 * removed
 */

bool _valve_safety_test(valve_t* valve, int sample)
{
    if (sample < (valve->watering_level - valve->safety_margin_lower)){
        valve->low_disable = true;
    }

    if (sample > (valve->watering_level)) {
        valve->low_disable = false;
    }

    if (sample > (valve->watering_level + valve->safety_margin_upper)){
        valve->high_disable = true;
    }

    DEBUG_PRINT("valve: watering level - %4d, sample value - %4d,",
            valve->watering_level, sample);
    DEBUG_PRINT("low disable - %1d, high disable - %1d\n",
            valve->low_disable, valve->high_disable);

    if (valve->low_disable || valve->high_disable) {
        return false;
    }

    return true;
}

void hygrometer_read(hygrometer_t* dev, int16_t* val)
{
    *val = adc_sample(dev->params->read_pin, RES);
}

int hygrometer_init(hygrometer_t* dev, const hygrometer_params_t* params)
{
    int ret;

    if ((ret = adc_init(params->read_pin)) < 0) {
        return ret;
    }

    dev->params = params;

    return 0;
}

void s_and_a_water(valve_t* valve)
{

    int moisture_level = 0;

    if (!valve->control_sensor) {
        for (uint8_t i = 0; i < SENSOR_NUMOF; i++) {
            moisture_level += sensors[i].raw_data;
        }
        moisture_level = moisture_level / SENSOR_NUMOF;
    }
    else {
        moisture_level = valve->control_sensor->raw_data;
    }

    if ((moisture_level > valve->watering_level) &&
         _valve_safety_test(valve, moisture_level)) {

        gpio_set(valve->control_pin);
        xtimer_sleep(valve->watering_time);
        gpio_clear(valve->control_pin);
    }

}

void s_and_a_sensor_update(sensor_t* sensor)
{
    /* TODO: my sensor_t looks a bit like a dev_t. Can I replace
     * this by essentially writing a driver in typical RIOT format for the ones
     * that aren't merged?
     */

    /* TODO: put failure tests around these in case any data reading fails */
    switch ( sensor->sensor_type ) {
        case SENSOR_T_HDC1000_TEMP:
            hdc1000_read_cached((const hdc1000_t *)sensor->dev, &(sensor->raw_data), NULL);
            break;

        case SENSOR_T_HDC1000_HUM:
            hdc1000_read_cached((const hdc1000_t *)sensor->dev, NULL, &(sensor->raw_data));
            break;

        case SENSOR_T_TSL4531X:
            sensor->raw_data = tsl4531x_simple_read(sensor->dev);
            break;

        case SENSOR_T_DS18:
            ds18_get_temperature(sensor->dev, &(sensor->raw_data));
            break;

        case SENSOR_T_HYGROMETER:
            hygrometer_read(sensor->dev, &(sensor->raw_data));
            break;

        default:
            break;
    }
}

void s_and_a_hardware_test(void)
{
    /* Test the power line for the moisture sensors */
    puts("Testing moisture sensor power lines. Light on control boards should be blinking on and off.");
    for (int i = 0; i < 5; i++) {
        gpio_set(GLOBAL_POWER_PIN);
        xtimer_sleep(1);
        gpio_clear(GLOBAL_POWER_PIN);
        xtimer_sleep(1);
    }

    /* Leave the power on for the rest of the test. */
    gpio_set(GLOBAL_POWER_PIN);

    /* Test the valves */
    puts("Testing the valves. Valves should click on and off three times each.");
    for (uint8_t i = 0; i < VALVE_NUMOF; i++) {
        for (int j = 0; j < 3; j++) {
            gpio_set(valves[i].control_pin);
            xtimer_sleep(1);
            gpio_clear(valves[i].control_pin);
            xtimer_sleep(1);
        }
    }

    /* Test the sensors */
    puts("Testing the sensors. Check the readings are sensible.");
    for (uint8_t i = 0; i < SENSOR_NUMOF; i++) {
        s_and_a_sensor_update(&sensors[i]);

        float cents = (float)sensors[i].raw_data/100;
        switch ( sensors[i].data_type ) {
            case SENSOR_DATA_T_TEMP:
            case SENSOR_DATA_T_HUM:
                printf("%s: %f", sensors[i].name, cents);
                break;
            case SENSOR_DATA_T_UINT16:
                printf("%s: %d", sensors[i].name, sensors[i].raw_data);
                break;

            default:
                break;
        }
    }

    gpio_clear(GLOBAL_POWER_PIN);
}

int s_and_a_valve_init(valve_t* valve)
{
    return gpio_init(valve->control_pin, GPIO_OUT);
}

int s_and_a_sensor_init(sensor_t* sensor, data_t* data)
{
    assert(sensor);
    assert(data);

    int ret;

    switch ( sensor->sensor_type ) {
        case SENSOR_T_HDC1000_TEMP:
            ret = hdc1000_init(sensor->dev, &hdc1000_params[sensor->config_no]);
            break;

            /* TODO: currently the HDC1000 gets initialised twice, because it's
             * got two readings, not because it's two devices. How to do this
             * better? Also handle params[1], [2] etc.
             *
             * These issues might be better to be addressed when figuring out
             * how to make this application extensible to any possible sensor
             * (that's supported in RIOT), without the user having to touch the
             * code.
             */

        case SENSOR_T_HDC1000_HUM:
            ret = hdc1000_init(sensor->dev, &hdc1000_params[sensor->config_no]);
            break;

        case SENSOR_T_TSL4531X:
            ret = tsl4531x_init(sensor->dev, &tsl4531x_params[sensor->config_no]);
            break;

        case SENSOR_T_DS18:
            ret = ds18_init(sensor->dev, &ds18_params[sensor->config_no]);
            break;

        case SENSOR_T_HYGROMETER:
            ret = hygrometer_init(sensor->dev, &hygrometer_params[sensor->config_no]);
            break;

        default:
            ret = -1;
            break;
    }

    data->type = sensor->data_type;

    return ret;
}

void s_and_a_update_all(data_t* data)
{
    gpio_set(GLOBAL_POWER_PIN);
    xtimer_sleep(GLOBAL_STARTUP_TIME);

    /* Collect data from sensors */
    for (uint8_t i = 0; i < SENSOR_NUMOF; i++) {
        assert(data + i);
        s_and_a_sensor_update(&sensors[i]);
        (data + i)->raw = sensors[i].raw_data;
    }
/* TODO: make sure there's a compile time check that data[] and sensors[] are
 * the same size, eg by using the same variable to declare
 */

#ifdef WATERING_ON
    for (uint8_t i = 0; i < VALVE_NUMOF; i++) {
        /* TODO: why am I passing around &valves when it's defined in module
         * level? Get the architecture clear.
         */
        s_and_a_water(&valves[i]);
    }
#endif

    gpio_clear(GLOBAL_POWER_PIN);
}

void s_and_a_init_all(data_t* data)
{
    int ret;

    if ((ret = gpio_init(GLOBAL_POWER_PIN, GPIO_OUT)) < 0) {
        printf("Couldn't initialise global power pin: %d\n", ret);
    }

    for (uint8_t i = 0; i < SENSOR_NUMOF; i++) {
        if ((ret = s_and_a_sensor_init(&sensors[i], data + i)) < 0) {
            printf("Couldn't initialise sensor %d: %d\n", i, ret);
        }
    }

    for (uint8_t i = 0; i < VALVE_NUMOF; i++) {
        if ((ret = s_and_a_valve_init(&valves[i])) < 0) {
            printf("Couldn't initialise valve %d: %d\n", i, ret);
        }
    }
}
