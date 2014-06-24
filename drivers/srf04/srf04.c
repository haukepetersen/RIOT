
#include "vtimer.h"

#include "srf04.h"


#define SPEED_OF_SOUND      (340290U)       /* speed of sound in mm per second */

void pulse_cb(int pw);

void trigger(srf04_t *dev);


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
