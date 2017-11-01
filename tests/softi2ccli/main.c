/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Manual test application for GPIO peripheral drivers
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "shell.h"
#include "periph/gpio.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"


#define TEST(x)     if (x != 0) {puts("[FAIL]"); return -1;}

#define SDA         GPIO_PIN(1,0)
#define SCL         GPIO_PIN(1,1)

#define MODE_OUT    GPIO_OUT
#define MODE_IN     GPIO_IN

static inline int high(gpio_t pin)
{
    return gpio_read(pin);
}

static inline int low(gpio_t pin)
{
    return !gpio_read(pin);
}

void i2c_init(void)
{
    gpio_init(SDA, MODE_IN);
    gpio_init(SCL, MODE_IN);
    DEBUG("i2c: SDA and SCL initialized\n");
}

 void i2c_wait_clear(void)
{
    while (low(SDA) || low(SCL)) {}
    DEBUG("i2c: clear\n");
}

 int i2c_expect_start(void)
{
    while (high(SDA) && high(SCL)) {}
    if (low(SCL)) {
        return -1;
    }
    while (low(SDA) && high(SCL)) {}
    if (high(SDA)) {
        return -1;
    }
    DEBUG("i2c: start condition received\n");
    return 0;
}

int i2c_expect_addr(uint8_t addr)
{
    uint8_t in = 0;

    if (high(SDA) || high(SCL)) {
        return -1;
    }
    for (int i = 0; i < 8; i++) {
        while (low(SCL)) {}
        in << 1;
        in |= (gpio_read(SDA)) ? 1 : 0;
        while (high(SCL)) {}
    }
    gpio_init(SDA, MODE_OUT);
    gpio_clear(SDA);



    DEBUG("i2c: address received\n");
    return 0;
}

static int i2c_nack(void)
{
    gpio_init()
}

static int cmd_expect_addr8(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s <addr> <R/W>\n", argv[0]);
        return 1;
    }

    uint8_t addr = (uint8_t)(atoi(argv[1]) << 1);
    if (argv[2][0] == 'R') {
        addr |= 0x1;
    }

    i2c_wait_clear();
    TEST(i2c_expect_start());
    TEST(i2c_expect_addr(addr));

    puts("[SUCCESS]");
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "addr", "test receiving of 7-bit address", cmd_expect_addr8 },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("Soft-I2C slave test\n");

    i2c_init();

    /* start the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
