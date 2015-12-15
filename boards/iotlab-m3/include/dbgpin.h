
#ifndef MM_H
#define MM_H

#include "cpu.h"
#include "periph/gpio.h"

#define PA          GPIO_PIN(PORT_B, 9)
#define PB          GPIO_PIN(PORT_C, 11)
#define PC          GPIO_PIN(PORT_A, 3)

#define MM_INIT     gpio_init(PA, GPIO_DIR_OUT, GPIO_NOPULL); \
                    gpio_init(PB, GPIO_DIR_OUT, GPIO_NOPULL)

#define MM1L        (GPIOB->BRR = (1 << 9))
#define MM1H        (GPIOB->BSRR = (1 << 9))
#define MM2L        (GPIOC->BRR = (1 << 11))
#define MM2H        (GPIOC->BSRR = (1 << 11))

#endif /* MM_H */
