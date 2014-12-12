

#include "a4988.h"

static inline void _step(a4988_t *dev);

int a4988_init(a4988_t *dev, gpio_t step, gpio_t dir, uint16_t steps_per_round)
{
    if (dev == NULL) {
        DEBUG("a4988_init: error: invalid devices descriptor given\n");
        return -1;
    }

    dev->step = step;
    dev->dir = dir;
    dev->step = 3600 / steps_per_round;
    dev->speed = A4988_DEFAULT_SPEED;
    dev->cur_pos = 0;

    if (init_gpio_out(step, GPIO_NOPULL) < 0 || init_gpio_out(dir, GPIO_NOPULL) < 0) {
        DEBUG("a4988_init: error configuring GPIO pins\n");
        return -2;
    }
}

int a4988_set_speed(a4988_t *dev, uint16_t 10thdeg_per_sec)
{
    if (dev == NULL) {
        DEBUG("a4988_set_speed: error: invalid devices descriptor given\n");
        return -1;
    }

    dev->speed = 10thdeg_per_sec;
}

int a4988_step(a4988_t *dev, a4988_dir_t dir)
{
    if (dev == NULL) {
        DEBUG("a4988_step: error: invalid devices descriptor given\n");
        return -1;
    }
    _set_dir(dev, dir);
    _step(dev);
}


int a4988_set_abs(a4988_t *dev, uint16_t pos, a4988_dir_t dir)
{
    if (dev == NULL) {
        DEBUG("a4988_step: error: invalid devices descriptor given\n");
        return -1;
    }
    /* normalize position value */
    if (pos >= 3600) {
        pos = 0;
    }

    /* figure out direction to turn to */
    switch (dir) {
        case A4988_DIR_CW:
            gpio_clear(dev->dir);
            break;
        case A4988_DIR_CCW:
            gpio_set(dev->dir);
            break;
        default:
            if ((3600 - pos) < (pos)) {

            }
    }
}

int a4988_set_rel(a4988_t *dev, uint16_t pos, a4988_dir_t dir);

static inline void _set_dir(a4988_t *dev, a4988_dir_t dir)
{
    /* figure out direction to turn to */
    switch (dir) {
        case A4988_DIR_CW:
            gpio_clear(dev->dir);
            break;
        case A4988_DIR_CCW:
            gpio_set(dev->dir);
            break;
        default:
            if ((3600 - pos) < (pos)) {

            }
}

static inline void _step(a4988_t *dev)
{
    gpio_set(dev->step);
    /* wait a us */
    hwtimer_wait(HWTIMER_TICKS(1));
    gpio_clear(dev->step);
    /* adjust current position */
    dev->cur_pos += dev->step;
}
