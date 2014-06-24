

#include "periph/gpio.h"
#include "periph/incap.h"

#define SRF04_MSG_DATA      (0x01)

typedef struct {
    gpio_t trigger;
    incap_t pulse;
} srf04_t;


int srf04_init_auto(void);


int srf04_init(srf04_t *dev, gpio_t trigger, incap_t pulse);


int srf04_measure_once(srf04_t *dev);

int srf04_measure(srf04_t *dev);

int srf04_stop(srf04_t *dev);

/**
 * @brief Sample a single distance value from the given sensor
 * 
 * This function will block until the value is ready, which will take
 * from 
 * 
 * @param[in] dev       the device to read from
 * 
 * @return              the distance value in mm
 * @return              -1 on error
 */
int srf04_sample(srf04_t dev);