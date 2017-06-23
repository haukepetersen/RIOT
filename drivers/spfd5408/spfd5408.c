

#include "assert.h"
#include "xtimer.h"

#include "spfd5408.h"
#include "internal.h"

int spfd5408_init(spfd5408_t *dev, const spfd5408_params_t *params)
{
    assert(dev && params);

    dev->p = params;

    /* initialize pins */
    gpio_init(dev->p->rst, GPIO_OUT);
    gpio_set(dev->p->rst);
    gpio_init(dev->p->cs, GPIO_OUT);
    gpio_set(dev->p->cs);
    gpio_init(dev->p->rs, GPIO_OUT);
    gpio_init(dev->p->wr, GPIO_OUT);
    gpio_init(dev->p->rd, GPIO_OUT);
    for (int i = 0; i < 8; i++) {
        gpio_init(dev->p->dat[i], GPIO_OUT);
    }

    /* reset display */
    gpio_clear(dev->p->rst);
    xtimer_usleep(TO_RESET);
    gpio_set(dev->p->rst);

    return SPFD5408_OK;
}

void spfd5400_line(spfd5408_t *dev, int ax, int ay, int bx, int by)
{
}
