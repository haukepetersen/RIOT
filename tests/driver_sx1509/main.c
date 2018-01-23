/*
 * Copyright (C) 2018 Freie Universität Berlin
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
 * @brief       Manual test application for the SX1509 driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "shell.h"
#include "sx150x.h"
#include "sx150x_params.h"

static sx150x_t sx;

static int get_pin(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <pin>\n", argv[0]);
        return -1;
    }
    int pin = atoi(argv[1]);
    if ((pin < 0) || (pin >= (int)SX150X_PIN_NUMOF)) {
        return -1;
    }
    return pin;
}

static int init_out(int argc, char **argv)
{
    int pin = get_pin(argc, argv);
    if (pin < 0) {
        return 1;
    }

    printf("initializing pin %u as output\n", pin);
    sx150x_gpio_init(&sx, (unsigned)pin, GPIO_OUT);
    return 0;
}

static int set(int argc, char **argv)
{
    int pin = get_pin(argc, argv);
    if (pin < 0) {
        return 1;
    }

    printf("setting pin %i\n", pin);
    sx150x_gpio_set(&sx, pin);
    return 0;
}

static int clear(int argc, char **argv)
{
    int pin = get_pin(argc, argv);
    if (pin < 0) {
        return 1;
    }

    printf("clearing pin %i\n", pin);
    sx150x_gpio_clear(&sx, pin);
    return 0;
}

static int init_all(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (unsigned i = 0; i < SX150X_PIN_NUMOF; i++) {
        sx150x_gpio_init(&sx, i, GPIO_OUT);
    }
    return 0;
}

static int set_all(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (unsigned i = 0; i < SX150X_PIN_NUMOF; i++) {
        sx150x_gpio_set(&sx, i);
    }
    return 0;
}

static int clear_all(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (unsigned i = 0; i < SX150X_PIN_NUMOF; i++) {
        sx150x_gpio_clear(&sx, i);
    }
    return 0;
}

static int set_led(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (unsigned i = 5; i < 8; i++) {
        sx150x_gpio_set(&sx, i);
    }
    for (unsigned i = 13; i < 16; i++) {
        sx150x_gpio_set(&sx, i);
    }
    return 0;
}

static int clear_led(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (unsigned i = 5; i < 8; i++) {
        sx150x_gpio_clear(&sx, i);
    }
    for (unsigned i = 13; i < 16; i++) {
        sx150x_gpio_clear(&sx, i);
    }
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "init_out", "init as output (push-pull mode)", init_out },
    { "set", "set pin to HIGH", set },
    { "clear", "set pin to LOW", clear },
    { "init_all", "initialize all pins", init_all },
    { "set_all", "set all pins", set_all },
    { "clear_all", "clear all pin", clear_all },
    { "set_led", "set all pins", set_led },         /* REMOVE! */
    { "clear_led", "clear all pin", clear_led },    /* REMOVE! */
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("Test application for the SX1509 device driver\n");

    printf("Initializing SX1509...");
    if (sx150x_init(&sx, &sx150x_params[0]) != SX150X_OK) {
        printf("\t[FAILED]\n");
        return 1;
    }
    puts("\t[OK]\n");

    /* start the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
