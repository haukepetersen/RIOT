typedef struct {
    uint16_t watering_level;
    bool low_disable;
    bool high_disable;
} valve_t;

static const valve_t valves[] = 
{
    /* valve config here */
}

static const sensor_t sensors[] =
{
    /* sensor config here */
}


#define RES             ADC_RES_10BIT

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
static hygrometer_t hygrometer_dev;

#ifdef VALVE_WATERING_ON
bool _valve_safety_test(valve_t valve)
{
    /* TODO: figure out how the safety/control of the valves will look,
     * so that each valve can operate based on either individual sensor
     * readings or an average. (Or other arbitrary function, if that's
     * possible).
     * Maybe this would look something like:
    int comparison_val = valve.determine_comparison_value(sensors);
     * but I need to think about whether the idea of storing a "comparison
     * value" is really what I want
    */

    if (sample < (valve.watering_level - SAFETY_LOWER)){
        valve.low_disable = true;
    }

    if (sample > (valve.watering_level)) {
        valve.low_disable = false;
    }

    if (sample > (valve.watering_level + SAFETY_UPPER)){
        valve.high_disable = true;
    }

    DEBUG_PRINT("valve - %4d, watering level - %4d, sample value - %4d,",
            valve_no, valve.watering_level, sample);
    DEBUG_PRINT("low disable - %1d, high disable - %1d\n",
            valve.low_disable, valve.high_disable);

    if (valve.low_disable || valve.high_disable) {
        return false;
    }

    return true;
}
#endif

int hygrometer_read(hygrometer_t* dev, int val)
{
    gpio_set(dev->power_pin);
    xtimer_sleep(dev->startup_time);
    val = adc_sample(dev->read_pin, RES);
    gpio_clear(dev->power_pin);
}

int hygrometer_init(hygrometer_t* dev)
{
    int ret;

    if ((ret = gpio_init(dev->power_pin, GPIO_OUT)) < 0) {
        return ret;
    }
    
    if ((ret = adc_init(dev->read_pin)) < 0) {
        return ret;
    }
    
    return 0;
}

void s_and_a_water(valve_t valve){

    if ((moisture_level > valve.watering_level) &&
         _valve_safety_test(valve)) {

        gpio_set(valve.control_pin);
        xtimer_sleep(valve.watering_time);
        gpio_clear(valve.control_pin);
    }

}

void s_and_a_sensor_update(sensor_t sensor)
{
    /* TODO: my sensor_t looks a bit like a dev_t. Can I replace
     * this by essentially writing a driver in typical RIOT format for the ones
     * that aren't merged?
     */

    /* TODO: put failure tests around these in case any data reading fails */
    switch ( sensor.type ) {
        case HDC1000_TEMP:	
            hdc1000_read_cached((const hdc1000_t *)&hdc1000_dev, &sensor.val, NULL);
            break;

        case HDC1000_HUMID:	
            hdc1000_read_cached((const hdc1000_t *)&hdc1000_dev, NULL, &sensor.val);
            break;

        case TSL4531X:	
            sensor.val = tsl4531x_simple_read(&tsl4531x_dev);
            break;

        case DS18:	
            ds18_get_temperature(&ds18_dev, &sensor.val);
            break;

        case HYGROMETER:	
            hygrometer_read(&hygrometer_dev, &sensor.val);
            break;

        default:	
            break;
    } 
}

int s_and_a_valve_init(valve_t valve)
{
    return gpio_init(valve.control_pin, GPIO_OUT);
}

int s_and_a_sensor_init(sensor_t sensor)
{
    switch ( sensor.type ) {
        case HDC1000_TEMP:	
            hdc1000_init(&hdc1000_dev, &hdc1000_params[0]);
            break;

            /* TODO: currently the HDC1000 gets initialised twice, because it's
             * got two readings, not because it's two devices. How to do this
             * better?
             */

        case HDC1000_HUMID:	
            hdc1000_init(&hdc1000_dev, &hdc1000_params[0]);
            break;

        case TSL4531X:	
            tsl4531x_init(&tsl4531x_dev, &tsl4531x_params[0]);
            break;

        case DS18:	
            ds18_init(&ds18_dev, &ds18_params[0]);
            break;

        case HYGROMETER:	
            hygrometer_init(&hygrometer_dev);
            break;

        default:	
            break;
    } 
}

void s_and_a_init_all(void)
{
    int ret;

    for (int i = 0, i < SENSOR_NUMOF; i++) {
        if ((ret = s_and_a_sensor_init(sensors[i])) < 0) {
            printf("Couldn't initialise sensor %d", i);
        }
    }

    for (int i = 0, i < VALVE_NUMOF; i++) {
        if ((ret = s_and_a_valve_init(valves[i])) < 0) {
            printf("Couldn't initialise valve %d", i);
        }
    }
}
