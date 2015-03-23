

#include <stdint.h>

#include "periph/gpio.h"

/**
 * @brief Define the default speed as 1 round per second
 */
#define A4988_DEFAULT_SPEED     3600

typedef struct {
    gpio_t step;
    gpio_t dir;
    uint16_t steps_per_round;
    uint16_t speed;
    uint16_t cur_pos;
    uint16_t set_pos;
}

typedef enum {
    A4988_DIR_CW    = 0,
    A4988_DIR_CCW   = 1,
    A4988_DIR_STOP  = 2
} a4988_dir_t;


int a4988_init(a4988_t *dev, gpio_t step, gpio_t dir, uint16_t steps_per_round);

int a4988_set_dir(a4988_t *dev, a4988_dir_t dir)

int a4988_step(a4988_t *dev, a4988_dir_t dir);

int a4988_step


int a4988_step_bw(a4988_t *dev);

int a4988_set_speed(a4988_t *dev, uint16_t 10thdeg_per_sec);

int a4988_set_abs(a4988_t *dev, int16_t pos, a4988_dir_t dir);

int a4988_set_rel(a4988_t *dev, int16_t pos, a4988_dir_t dir);
