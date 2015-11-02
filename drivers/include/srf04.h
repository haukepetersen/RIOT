
#include <stdint.h>

#include "mutex.h"
#include "periph/gpio.h"


typedef struct {
    gpio_t trigger;
    gpio_t echo;
    uint32_t pulse_len;
    mutex_t lock;
} srf04_t;


typedef enum {
    SRF04_MODE_INCH,
    SRF04_MODE_MM,
    SRF04_MODE_MS
} srf04_mode_t;



int srf04_init(srf04_t *dev, gpio_t trigger_pin, gpio_t echo_pin);

uint16_t srf04_get_distance(srf04_t *dev, srf04_mode_t mode);
