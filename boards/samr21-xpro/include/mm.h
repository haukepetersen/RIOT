






#ifndef MMABC_H
#define MMABC_H

#include "cpu.h"
#include "periph/gpio.h"


#define MM_INIT         gpio_init(GPIO_PIN(PA,6), GPIO_DIR_OUT, GPIO_NOPULL); \
                        gpio_init(GPIO_PIN(PA,7), GPIO_DIR_OUT, GPIO_NOPULL)

#define MM1L            (PORT->Group[0].OUTCLR.reg = (1 << 6))
#define MM1H            (PORT->Group[0].OUTSET.reg = (1 << 6))
#define MM2L            (PORT->Group[0].OUTCLR.reg = (1 << 7))
#define MM2H            (PORT->Group[0].OUTSET.reg = (1 << 7))
#define MM2T            (PORT->Group[0].OUTTGL.reg = (1 << 7))

#endif /* MMABC_H */
