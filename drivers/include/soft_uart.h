


#ifndef SOFT_UART_H
#define SOFT_UART_H

#include "xtimer.h"
#include "periph/uart2.h"

typedef enum {
    SOFT_UART_8N1,
    SOFT_UART_8N2,
} soft_uart_mode_t;

typedef struct {
    uint32_t t_a;
    uint32_t t_b;
} soft_uart_t;

typedef struct {
    gpio_t pin_tx;
    gpio_t pin_rx;
    uint32_t baudrate;
    soft_uart_mode_t;
} soft_uart_conf_t;

#endif /* SOFT_UART_H */
