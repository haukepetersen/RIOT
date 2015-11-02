

#include "periph/gpio.h"





#define MM_INIT()       gpio_init(GPIO_PIN(P5,1), GPIO_DIR_OUT, GPIO_NOPULL); \
                        gpio_init(GPIO_PIN(P5,2), GPIO_DIR_OUT, GPIO_NOPULL)

#define MM_1L()         (P5OUT &= ~0x1)
#define MM_1H()         (P5OUT |= 0x1)
#define MM_2L()         (P5OUT &= ~0x2)
#define MM_2H()         (P5OUT |= 0x2)
