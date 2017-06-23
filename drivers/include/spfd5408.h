





#ifndef SPFD5408_H
#define SPFD5408_H

#include "periph/gpio.h"

enum {
    SPFD5408_OK  = 0,
    SPFD5408_SOMEERROR = -1
};

typedef struct {
    int width;
    int height;
    gpio_t rst;
    gpio_t cs;
    gpio_t rs;
    gpio_t wr;
    gpio_t rd;
    gpio_t dat[8];
} spfd5408_params_t;

typedef struct {
    const spfd5408_params_t *p;
} spfd5408_t;

int spfd5408_init(spfd5408_t *dev, const spfd5408_params_t *params);

void spfd5400_line(spfd5408_t *dev, int ax, int ay, int bx, int by);


#endif /* SPFD5408_H */
/** @} */
