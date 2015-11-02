
#include "vtimer.h"

#include "srf04.h"

#define SRF04_MSG_DATA      (0x01)

#define SPEED_OF_SOUND      (340290U)       /* speed of sound in mm per second */

/**
 * @brief   The datasheet tells us, that the trigger pulse should be 10us wide
 */
#define TRIGGER_DELAY       (10U)


#define NOTHING_IN_RANGE    (32000U)

static void echo_cb(void *arg)
{
    srf04_t *dev = (srf04_t *)arg;

    uint32_t now = xtimer_now();

    if (gpio_get(dev->echo)) {
        dev->pulse = now;
    }
    else {
        if (now > dev->pulse) {
            dev->pulse = now - dev->pulse;
        }
        else {
            dev->pulse = now + (0xffffffff - dev->pulse);
        }
        mutex_unlock(&dev->lock);
    }
}

int srf04_init(srf04_t *dev, gpio_t trigger_pin, gpio_t echo_pin)
{
    /* remember pins and initialize mutex */
    dev->trigger = trigger_pin;
    dev->echo = echo_pin;
    mutex_init(&dev->lock);
    mutex_lock(&dev->lock);

    /* initialize pins */
    gpio_init(dev->trigger, GPIO_DIR_OUT, GPIO_NOPULL);
    gpio_clear(dev->trigger);
    gpio_init_int(dev->echo, GPIO_FLANK_BOTH, GPIO_NOPULL, echo_cb, dev);

    return 0;
}

uint16_t srf04_get_distance(srf04_t *dev, srf04_mode_t mode)
{
    uint32_t res;

    /* send trigger signal */
    gpio_set(dev->trigger);
    xtimer_usleep(TRIGGER_DELAY);
    gpio_clear(dev->trigger);

    /* wait for echo pulse */
    mutex_lock(&dev->lock);
    /* check if anything was in range */
    if (dev->pulse >= NOTHING_IN_RANGE) {
        return 0;
    }
    /* compute actual distance value */
    switch (mode) {
        case SRF04_MODE_INCH:

    }
}




#if SRF04_TRIGGER && SRF04_PULSE && SRF04_DEV
int srf04_init_auto(void)
{

}
#endif




int srf04_init(srf04_t *dev, gpio_t trigger, incap_t pulse)
{
    gpio_init_out(trigger, GPIO_NOPULL);
    incap_init(pulse, 1, INCAP_RISING, incap_cb);
}

int srf04_measure_once(srf04_t *dev)
{
    dev->continuous = FALSE;
    trigger(dev);

}

int srf04_measure(srf04_t)
{
    dev->continuous = TRUE;
}

int srf04_stop(srf04_t dev)
{
    incap_disable(dev->pulse);
}

void pulse_cb(int pw)
{
    msg_t msg;

    if (!dev->continuous) {
        incap_disable(dev->pulse);
    }

    /* pw holds the response pulse width in us.
       To extract the distance in mm, d=pw/1000/s_sound/2 */
    uint32_t distance = pw / 2 / 1000000 * SPEED_OF_SOUND;

    /* send distance to interested thread */
    msg.type = SRF04_MSG_DATA
    msg.value = distance;
    msg_send(&msg, dev->pid, false);
}

void trigger(srf04_t *dev)
{
    /* send out trigger signal */
    gpio_set(dev->trigger);
    vtimer_usleep(10);       /* send a 10us trigger signal */
    gpio_clear(dev->trigger);
}
