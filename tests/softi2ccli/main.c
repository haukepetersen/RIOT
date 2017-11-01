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
 * @todo        - Put timeout on tests
 *
 * @file
 * @brief       Manual test application for GPIO peripheral drivers
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "shell.h"
#include "periph/gpio.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

#define FLAG_READ       (0x01)
#define FLAG_WRITE      (0x00)


#define VALIDATE(x)     if (x < 0) { goto fail; }

#define SDA         GPIO_PIN(1,8)
#define SCL         GPIO_PIN(1,9)

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
    DEBUG("i2c: bus clear\n");
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
    return 0;
}

int i2c_expect_stop(void)
{
    while (low(SCL)) {}
    if (high(SDA)) {
        DEBUG("i2c_expect_stop: SDA not low\n");
        return -1;
    }
    while (low(SDA)) {}
    if (low(SCL)) {
        DEBUG("i2c_expect_stop: SCL not low\n");
        return -1;
    }
    return 0;
}

int i2c_expect_byte_in(void)
{
    int in = 0;

    /* pre: SCL low */
    if (high(SCL)) {
        DEBUG("i2c_expect_byte_in: pre not met\n");
        return -1;
    }

    for (int i = 0; i < 8; i++) {
        while (low(SCL)) {}
        in <<= 1;
        in |= (gpio_read(SDA)) ? 1 : 0;
        while (high(SCL)) {}
    }
    return (int)in;
}

int i2c_expect_ack(void)
{
    /* pre: SCL low */
    if (high(SCL)) {
        DEBUG("i2c_expect_ack: SCL not low\n");
        return -1;
    }

    while (low(SCL)) {}
    if (gpio_read(SDA)) {
        DEBUG("i2c_expect_ack: got NACK\n");
        return -1;
    }
    while (high(SCL)) {}

    return 0;
}

int i2c_expect_nack(void)
{
    /* pre: SCL low */
    if (high(SCL)) {
        DEBUG("i2c_expect_nack: SCL not low\n");
        return -1;
    }

    while (low(SCL)) {}
    if (!gpio_read(SDA)) {
        DEBUG("i2c_expect_nack: got ACK\n");
        return -1;
    }
    while (high(SCL)) {}

    return 0;
}

int i2c_byte_out(uint8_t data)
{
    /* pre: SCL low */
    if (high(SCL)) {
        DEBUG("i2c_byte_out: pre not met\n");
        return -1;
    }

    gpio_init(SDA, MODE_OUT);

    for (int i = 0; i < 8; i++) {
        gpio_write(SDA, data & 0x80);
        data <<= 1;
        while (low(SCL)) {}
        while (high(SCL)) {}
    }

    gpio_init(SDA, MODE_IN);
    return 0;
}

int i2c_ack(void)
{
    /* entry: SDA -> x, SCL -> low */
    if (high(SCL)) {
        DEBUG("i2c/ack: SCL not low\n");
        return -1;
    }
    gpio_init(SDA, MODE_OUT);
    gpio_clear(SDA);
    while (low(SCL)) {}
    while (high(SCL)) {}
    gpio_set(SDA);
    gpio_init(SDA, MODE_IN);
    return 0;
}

int i2c_nack(void)
{
    /* entry: SDA -> x, SCL -> low */
    if (high(SCL)) {
        DEBUG("i2c/nack: SCL not low\n");
        return -1;
    }
    /* wait for the next SCL pulse without touching SDA (keep in high) */
    while (low(SCL)) {}
    while (high(SCL)) {}
    return 0;
}

static int cmd_state(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    char *sda = (gpio_read(SDA)) ? "high" : "low";
    char *scl = (gpio_read(SCL)) ? "high" : "low";

    printf("SDA %s, SCL %s\n", sda, scl);
    return 0;
}

static int cmd_expect_addr7(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s <r|w> <addr>\n", argv[0]);
        return 1;
    }

    int addr_in;
    int addr = ((atoi(argv[2]) & 0xff) << 1);
    if (argv[1][0] == 'r') {
        addr |= 0x1;
    }

    printf("expect: START, 7-bit addr (%i), !NACK!, STOP\n", (int)addr);

    i2c_wait_clear();
    VALIDATE(i2c_expect_start());
    VALIDATE((addr_in = i2c_expect_byte_in()));
    VALIDATE(i2c_nack());
    VALIDATE(i2c_expect_stop());

    if (addr_in != addr) {
        printf("err: incoming address does not match (%i vs %i)\n",
               (int)addr, (int)addr_in);
        goto fail;
    }

    puts("[SUCCESS]");
    return 0;

fail:
    puts("[FAIL]");
    return 1;
}

static int cmd_expect_write7(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s <addr> <data>\n", argv[0]);
        return 1;
    }

    int addr = (((atoi(argv[1]) & 0xff) << 1) | FLAG_WRITE);
    size_t data_len = strlen(argv[2]);

    printf("Expect writing to 7-bit address - addr %i, %i byte\n",
            (int)addr, (int)data_len);

    i2c_wait_clear();
    VALIDATE(i2c_expect_start());
    int addr_in = i2c_expect_byte_in();
    VALIDATE(addr_in);
    if (addr_in != addr) {
        VALIDATE(i2c_nack());
        VALIDATE(i2c_expect_stop());
        printf("err: incoming address does not match\n");
        goto fail;
    }
    VALIDATE(i2c_ack());
    for (size_t i = 0; i < data_len; i++) {
        int in = i2c_expect_byte_in();
        VALIDATE(in);
        if (in == (int)argv[2][i]) {
            VALIDATE(i2c_ack());
        }
        else {
            VALIDATE(i2c_nack());
            VALIDATE(i2c_expect_stop());
            goto fail;
        }
    }
    VALIDATE(i2c_expect_stop());

    puts("[SUCCESS]");
    return 0;

fail:
    puts("[FAIL]");
    return 1;
}

static int cmd_expect_read7(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s <addr> <data>\n", argv[0]);
        return 1;
    }

    int addr_in;
    int addr = (((atoi(argv[1]) & 0xff) << 1) | FLAG_READ);
    size_t data_len = strlen(argv[2]);

    i2c_wait_clear();
    VALIDATE(i2c_expect_start());
    addr_in = i2c_expect_byte_in();
    VALIDATE(addr_in);
    if (addr_in != addr) {
        VALIDATE(i2c_nack());
        VALIDATE(i2c_expect_stop());
        printf("err: incoming address does not match\n");
        goto fail;
    }
    VALIDATE(i2c_ack());

    for (size_t i = 0; i < data_len; i++) {
        VALIDATE(i2c_byte_out((uint8_t)argv[2][i]));
        if (i < (data_len - 1)) {
            VALIDATE(i2c_expect_ack());
        }
        else {
            VALIDATE(i2c_expect_nack());
        }
    }
    VALIDATE(i2c_expect_stop());

    puts("[SUCCESS]");
    return 0;

fail:
    puts("[FAIL]");
    return 1;
}

static const shell_command_t shell_commands[] = {
    { "state", "show the current bus status", cmd_state },
    { "addr", "test receiving of 7-bit address", cmd_expect_addr7 },
    { "write", "test master write to 7-bit address", cmd_expect_write7 },
    { "read", "test master read from 7-bit address", cmd_expect_read7 },
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
