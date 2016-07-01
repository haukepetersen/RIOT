/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       xtimer_msg test application
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <stdlib.h>
#include <stdio.h>

#include "fmt.h"
#include "shell.h"
#include "xtimer.h"

#include "ds18b20.h"

#define DEVICE_LIMIT        (10U)

static onewire_t owi = { GPIO_UNDEF, GPIO_OD_PU, GPIO_IN_PU };

static onewire_rom_t devlist[DEVICE_LIMIT];

static ds18b20_t dsdev;


static int cmd_init(int argc, char **argv)
{
    int portsel, pinsel, modesel;
    onewire_params_t params;

    if (argc < 3) {
        printf("usage: %s <port num> <pin num> [mode]\n", argv[0]);
        puts("\tmode: 0: open drain with pull-up (default)");
        puts("\t      1: open-drain without pull resistor");
        puts("\t      2: push pull output (external transistor needed)");
        return 1;
    }

    portsel = atoi(argv[1]);
    pinsel = atoi(argv[2]);
    params.pin = GPIO_PIN(portsel, pinsel);
    params.mode = GPIO_OD_PU;
    if (argc > 3) {
        modesel = atoi(argv[3]);
        switch (modesel) {
            case 0:
                break;
            case 1:
                params.mode = GPIO_OD;
                break;
            case 2:
                params.mode = GPIO_OUT;
                break;
            default:
                puts("unable to parse mode");
                return 1;
        }
    }

    printf("Initializing 1-Wire bus at GPIO_PIN(%i, %i)\n", portsel, pinsel);

    /* initialize the one wire bus */
    onewire_setup(&owi, &params);
    if (onewire_init(&owi) != ONEWIRE_OK) {
        puts("Error initializing the bus: open-drain pin mode not applicable\n");
        owi.pin = GPIO_UNDEF;
        return 1;
    }
    puts("Bus successfully initialized\n");

    return 0;
}

static int cmd_dsinit(int argc, char **argv)
{
    int devnum;

    if (argc < 2) {
        printf("usage: %s <dev num>\n", argv[0]);
        return 1;
    }

    devnum = atoi(argv[1]);
    if (devnum < 0 || devnum >= DEVICE_LIMIT) {
        puts("error: given device number out of range");
        return 1;
    }
    if (devlist)

    return 0;
}

static int cmd_dsread(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    char res[8];

    uint16_t temp = ds18b20_get_temp(&dsdev);
    fmt_s16_dfp(out, temp, 3);
    printf("Temperature is %s°C\n", out);

    return 0;
}

static int cmd_list(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (owi.pin == GPIO_UNDEF) {
        puts("error: onewire bus is not initialized");
        return 1;
    }

    memset(devlist, 0, DEVICE_LIMIT * sizeof(devlist[0]));
    onewire_search(&owi, devlist, DEVICE_LIMIT);

    puts("List of 1-wire devices connected to the bus:");
    for (int i = 0; i < DEVICE_LIMIT; i++) {
        if (devlist[i].u64 != 0) {
            printf("[%2i] ROM code %08x%08x\n",
                    devlist[i].u32[0], devlist[i].u32[1]);
        }
        else {
            break;
        }
    }

    return 0;
}

static int cmd_test(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (owi.pin == GPIO_UNDEF) {
        puts("error: onewire bus is not initialized");
        return 1;
    }

    /* generate reset sequence */
    if (onewire_reset(&owi) == ONEWIRE_NODEV) {
        puts("no device present on the bus");
        return 1;
    }

    /* read ROM command */
    onewire_write_byte(&owi, 0x33);

    uint8_t rom[8];
    memset(rom, 0, 8);
    onewire_read(&owi, rom, 8);

    printf("ROM found is ");
    for (int i = 0; i < 8; i++) {
        printf("0x%02x ", (int)rom[i]);
    }
    puts("");


    uint8_t pad[9];
    memset(pad, 0, 9);

    onewire_reset(&owi);
    onewire_write_byte(&owi, 0x55);
    onewire_write(&owi, rom, 8);

    onewire_write_byte(&owi, 0xbe);
    onewire_read(&owi, pad, 9);
    printf("Scratchpad is ");
    for (int i = 0; i < 9; i++) {
        printf("0x%02x ", (int)pad[i]);
    }
    puts("");

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "init", "initialize OneWire interface on given pin", cmd_init },
    { "list", "list all connected devices", cmd_list },
    { "test", "foobar", cmd_test },
    { "dsinit", "initialize the given ds18b20 sensor", cmd_dsinit },
    { "dsread", "read temperature from sensor", cmd_dsread },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("1-Wire bus driver test application\n");

    memset(dsdev, 0, sizeof(ds18b20_t));
    memset(devlist, 0, DEVICE_LIMIT * sizeof(devlist[0]));

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
