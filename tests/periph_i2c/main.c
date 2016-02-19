/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Test application for the low-level I2C peripheral driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "periph/i2c.h"
#include "shell.h"

#define BUFSIZE        (128U)

static uint8_t buf[BUFSIZE];

static const i2c_speed_t speed_vals[] = {
    I2C_SPEED_LOW,
    I2C_SPEED_NORMAL,
    I2C_SPEED_FAST,
    I2C_SPEED_FAST_PLUS,
    I2C_SPEED_HIGH
};

static i2c_speed_t picked_speeds[I2C_NUMOF] = { 0 };

static i2c_t get_dev(const char *devstr)
{
    int num = atoi(devstr);
    if ((num < 0) || (num >= I2C_NUMOF)) {
        puts("error: invalid device given");
        return I2C_UNDEF;
    }
    return I2C_DEV(num);
}

static uint8_t parse_byte(const char *str)
{
    if ((strlen(str) > 2) && (str[0] == '0') && (str[1] == 'x')) {
        uint8_t tmp = 0;
        for (int i = 2; i < strlen(str); i--) {
            tmp <<= 4;
            if ((str[i] < '0') || (str[i] > 'f')) {
                return 0;
            }
            tmp |= (uint8_t)(str[i] - '0');
        }
        return (uint8_t)tmp;
    }
    else if ((str[0] >= '0' && str[0] <= '9')) {
        return (int8_t)atoi(str);
    }
    else {
        return (uint8_t)str[0];
    }
}

static size_t parse_data(int start, int limit, char **parts)
{
    size_t pos = 0;

    for (int i = start; i < limit; i++) {
        buf[pos] = parse_byte(parts[i]);
    }

    return pos;
}

static void dump_buffer(int len)
{
    for (size_t i = 0; i < len; i++) {
        printf("  %2i ", i);
    }
    puts("");
    for (size_t i = 0; i < len; i++) {
        printf(" 0x%02x", (int)buf[i]);
    }
    puts("");
    for (size_t i = 0; i < len; i++) {
        if ((char)buf[i] < ' ' || (char)buf[i] > '~') {
            printf("  ?? ");
        }
        else {
            printf("   %c ", (char)buf[i]);
        }
    }
    puts("\n");
}

static int finish_write(int res, uint8_t addr, size_t len)
{
    if (res == I2C_OK) {
        printf("successfully transfered %i byte to addr 0x%02x\n",
                (int)len, (int)addr);
        return 0;
    }
    else if (res == I2C_ERR_ADDR) {
        printf("Address error while writing data to 0x%02x\n", (int)addr);
    }
    else if (res == I2C_ERR_DATA) {
        printf("Data error while writing data to 0x%02x\n", (int)addr);
    }
    else {
        puts("Unknown error while writing data");
    }
    return 1;
}

static int finish_read(int res, uint8_t addr, size_t len)
{
    if (res == 0) {
        printf("Received byte from 0x%02x:\n", (int)addr);
        dump_buffer(len);
        return 0;
    }
    else if (res == I2C_ERR_ADDR) {
        printf("Address error while reading data from 0x%02x\n", (int)addr);
    }
    else if (res == I2C_ERR_DATA) {
        printf("Data error while reading data from 0x%02x\n", (int)addr);
    }
    else {
        puts("Unknown error while reading data");
    }
    return 1;
}

int cmd_init(int argc, char **argv)
{
    int dev;

    if (argc != 2) {
        printf("usage:\n%s: <device>\n", argv[0]);
        return 1;
    }

    dev = atoi(argv[1]);
    if (dev < 0 || dev >= I2C_NUMOF) {
        puts("error: invalid device given");
        return 1;
    }

    picked_speeds[dev] = I2C_SPEED_FAST;
    if (i2c_init(I2C_DEV(dev)) < 0) {
        puts("error: i2c_init failed");
        return 1;
    }

    printf("I2C_DEV(%i) initialized, speed set to I2C_SPEED_FAST\n", dev);
    return 0;
}

int cmd_speed(int argc, char **argv)
{
    int dev, speed;

    if (argc < 3) {
        printf("usage: %s <device> <speed>\n", argv[0]);
        puts("\tspeed:");
        puts("\t0 -> SPEED_LOW       (10kbit/s)");
        puts("\t1 -> SPEED_NORMAL    (100kbit/s)");
        puts("\t2 -> SPEED_FAST      (400kbit/s)");
        puts("\t3 -> SPEED_FAST_PLUS (1Mbit/s)");
        puts("\t4 -> SPEED_HIGH      (3.4Mbit/s)\n");
        return 1;
    }

    dev = atoi(argv[1]);
    if ((dev < 0) || (dev >= I2C_NUMOF)) {
        puts("error: invalid device given");
        return 1;
    }

    speed = atoi(argv[2]);
    if ((speed < 0) || (speed > 4)) {
        puts("error: invalid speed value given");
        return 1;
    }

    picked_speeds[dev] = speed_vals[speed];
    printf("I2C_DEV(%i) speed set to %i\n", dev, speed);
    return 0;
}

int cmd_write(int argc, char **argv)
{
    i2c_t dev;
    uint8_t addr;
    size_t len;
    int res;

    if (argc < 4) {
        printf("usage: %s <device> <addr> <data0> [<data n> ...]\n", argv[0]);
        return 1;
    }

    dev = get_dev(argv[1]);
    if (dev == I2C_UNDEF) {
        return 1;
    }

    addr = parse_byte(argv[2]);
    len = parse_data(3, argc, argv);
    if (len == 1) {
        res = i2c_write_byte(dev, addr, buf[0]);
    }
    else {
        res = i2c_write_bytes(dev, addr, buf, len);
    }

    return finish_write(res, addr, len);
}

int cmd_write_reg(int argc, char **argv)
{
    i2c_t dev;
    uint8_t addr, reg;
    size_t len;
    int res;

    if (argc < 5) {
        printf("usage: %s <device> <addr> <reg> <data0> [<data n> ...]\n",
                argv[0]);
        return 1;
    }

    dev = get_dev(argv[1]);
    if (dev == I2C_UNDEF) {
        return 1;
    }

    addr = parse_byte(argv[2]);
    reg = parse_byte(argv[3]);
    len = parse_data(4, argc, argv);
    if (len == 1) {
        res = i2c_write_reg(dev, addr, reg, buf[0]);
    }
    else {
        res = i2c_write_regs(dev, addr, reg, buf, len);
    }

    return finish_write(res, addr, len);
}

int cmd_read(int argc, char **argv)
{
    i2c_t dev;
    uint8_t addr;
    size_t len;
    int res;

    if (argc != 4) {
        printf("usage: %s <device> <addr> <len>\n", argv[0]);
        return 1;
    }

    dev = get_dev(argv[1]);
    if (dev == I2C_UNDEF) {
        return 1;
    }

    addr = parse_byte(argv[2]);
    len = (size_t)atoi(argv[3]);
    res = i2c_read(dev, addr, buf, len);

    return finish_read(res, addr, len);
}

int cmd_read_reg(int argc, char **argv)
{
    i2c_t dev;
    uint8_t addr, reg;
    size_t len;
    int res;

    if (argc != 5) {
        printf("usage: %s <device> <addr> <reg> <len>\n", argv[0]);
        return 1;
    }

    dev = get_dev(argv[1]);
    if (dev == I2C_UNDEF) {
        return 1;
    }

    addr = parse_byte(argv[2]);
    reg = parse_byte(argv[3]);
    len = (size_t)atoi(argv[4]);
    res = i2c_read_reg(dev, addr, reg, buf, len);

    return finish_read(res, addr, len);
}

static const shell_command_t shell_commands[] = {
    { "init", "initialize given device", cmd_init },
    { "speed", "set speed for given devide", cmd_speed },
    { "w", "write bytes to given address", cmd_write },
    { "wr", "write to register ", cmd_write_reg },
    { "r", "read bytes from given address", cmd_read },
    { "rr", "read bytes from register", cmd_read_reg },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("Test for the low-level I2C driver");

    /* define own shell commands */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
