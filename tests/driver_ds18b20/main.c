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

#include "onewire.h"
#include "ds18b20.h"

static onewire_t owi = { GPIO_UNDEF, GPIO_OD_PU, GPIO_IN_PU };
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
    ds18b20_params_t params;
    params.res = DS18B20_RES_10BIT;

    if (owi.pin == GPIO_UNDEF) {
        puts("error: onewire bus is not initialized");
        return 1;
    }

    if (argc < 2) {
        printf("usage: %s <rom> [res]\n", argv[0]);
        puts("\twhere res is resolution in bit [9-12], 10 is default\n");
        return 1;
    }

    /* set 1-wire bus */
    memcpy(&(params.bus), &owi, sizeof(owi));

    /* set ROM address */
    if (onewire_rom_from_str(&params.rom, argv[1]) != ONEWIRE_OK) {
        puts("error: unable to parse given ROM");
        return 1;
    }

    /* optionally set resolution */
    params.res = DS18B20_RES_10BIT;
    if (argc >= 3) {
        int resnum = atoi(argv[2]);
        switch (resnum) {
            case  9: params.res = DS18B20_RES_9BIT;  break;
            case 10: params.res = DS18B20_RES_10BIT; break;
            case 11: params.res = DS18B20_RES_11BIT; break;
            case 12: params.res = DS18B20_RES_12BIT; break;
            default:
                puts("error: given resolution is invalid");
                return 1;
        }
    }

    ds18b20_setup(&dsdev, &params);
    if (ds18b20_init(&dsdev) != DS18B20_OK) {
        puts("error: unable to initialize ds18b20 device\n");
        return 1;
    }
    puts("DS18B20 device initialization successful");

    return 0;
}

static int cmd_dsread(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    char res[8];

    int16_t temp = ds18b20_get_temp(&dsdev);
    fmt_s16_dfp(res, temp, 3);
    printf("Temperature is %s°C\n", res);

    return 0;
}

static int cmd_list(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    onewire_rom_t rom;
    int cnt = 0;

    if (owi.pin == GPIO_UNDEF) {
        puts("error: onewire bus is not initialized");
        return 1;
    }

    /* reset current rom */
    memset(&rom, 0, sizeof(onewire_rom_t));

    /* search for devices on the bus */
    puts("List of 1-wire devices connected to the bus:");
    if (onewire_search_first(&owi, &rom) == ONEWIRE_OK) {
        printf("[%2i] ROM ADDR: %02x%02x%02x%02x%02x%02x%02x%02x\n",
                    cnt, (int)rom.u8[0], (int)rom.u8[1], (int)rom.u8[2], (int)rom.u8[3],
                    (int)rom.u8[4], (int)rom.u8[5], (int)rom.u8[6], (int)rom.u8[7]);
        ++cnt;

        while (onewire_search_next(&owi, &rom) == ONEWIRE_OK) {
            printf("[%2i] ROM ADDR: %02x%02x%02x%02x%02x%02x%02x%02x\n",
                    cnt, (int)rom.u8[0], (int)rom.u8[1], (int)rom.u8[2], (int)rom.u8[3],
                    (int)rom.u8[4], (int)rom.u8[5], (int)rom.u8[6], (int)rom.u8[7]);
            ++cnt;
        }
    }
    printf("  -- %i devices found --\n", cnt);

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

    int16_t temp;
    temp = (pad[1] << 8 | pad[0]);
    printf("temperature is %i.%i\n", ((int)temp >> 4), ((int)temp & 0xf));

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

    memset(&dsdev, 0, sizeof(dsdev));

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
