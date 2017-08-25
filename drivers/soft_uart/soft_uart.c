

#include "assert.h"
#include "soft_uart.h"

#define CYCLE           (104)

static soft_uart_config_t *get(const void *cfg)
{
    return (soft_uart_config_t *)cfg;
}

static int init(const void *cfg, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    soft_uart_t *dev = (soft_uart_t *)cfg;
    assert(cfg);

    if (baudrate != 9600) {
        return UART_NOBAUD;
    }


    gpio_init(get(cfg)->pin_tx, GPIO_OUT);
    gpio_clear(get(cfg)->pin_tx);

    if (rx_cb) {
        dev->timer.callback = rx_sample;
        dev->timer.arg      = cfg;
        gpio_init_int(get(cfg)->pin_rx, GPIO_IN, GPIO_RISING, event, cfg);
        /* TODO save callback */
    }
}

static void write(const void *cfg, const uint8_t *data, size_t len)
{
    assert(cfg);

    for (size_t i = 0; i < len; i++) {
        /* start condition */
        gpio_set(get(cfg)->pin_tx);
        xtimer_usleep(CYCLE);
        /* shift out data */
        for (int bit = 0; bit < 8; bit++) {
            gpio_write(get(cfg)->pin_tx, (data[i] & (1 << bit)));
            xtimer_usleep(CYCLE);
        }
        /* stop condition */
        gpio_clear(get(cfg)->pin_tx);
        xtimer_usleep(CYCLE);
    }
}

static void rx_sample(void *cfg)
{
    soft_uart_t *dev = (soft_uart_t *)cfg;

    if (dev->bitcnt++ < 8) {
        xtimer_set(&dev->timer, CYCLE);
        dev->in <<= 1;
        if (gpio_read(dev->pin_rx)) {
            dev->in |= 1;
        }
    }
    else {
        gpio_init_int(dev->pin_rx, GPIO_IN, GPIO_RISING, event, cfg);
    }
}

static void event(void *cfg)
{
    soft_uart_t *dev = (soft_uart_t *)cfg;

    gpio_init(get(cfg)->pin_rx, GPIO_IN);

    dev->bitcnt = 0;
    dev->in = 0;
    xtimer_set(&dev->timer, CYCLE / 2);
}
