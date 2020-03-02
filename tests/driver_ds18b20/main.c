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

#define DEVICE_LIMIT        (10U)

static onewire_t owi = { GPIO_UNDEF, GPIO_OD_PU, GPIO_IN_PU };
static ds18b20_params_t _dslist[DEVICE_LIMIT];
static ds18b20_t _devlist[DEVICE_LIMIT];


static int _cmd_init(int argc, char **argv)
{
    int portsel, pinsel, modesel, res;
    onewire_params_t params;
    uint8_t dsres;

    if (argc < 3) {
        printf("usage: %s <port num> <pin num> [mode] [res]\n", argv[0]);
        puts("\tmode: 0: open drain with pull-up (default)");
        puts("\t      1: open-drain without pull resistor");
        puts("\t      2: push pull output (external transistor needed)");
        puts("\t res: 9, 10, 11, or 12-bit resolution, default is 12-bit");
        return 1;
    }

    portsel = atoi(argv[1]);
    pinsel = atoi(argv[2]);
    params.pin = GPIO_PIN(portsel, pinsel);
    params.mode = GPIO_OD_PU;

    if (argc >= 4) {
        modesel = atoi(argv[3]);
        switch (modesel) {
            case 1: params.mode = GPIO_OD;      break;
            case 2: params.mode = GPIO_OUT;     break;
            default:
                puts("err: unable to parse OneWire pin mode");
                return 1;
        }
    }

    if (argc >= 5) {
        res = atoi(argv[4]);
        if (res < 9 || res > 12) {
            puts("err: resolution must be 9, 10, 11, or 12");
            return 1;
        }
        dsres = res - 9;
    }

    /* initialize the one wire bus */
    printf("initializing OneWire bus on GPIO_PIN(%i, %i)...\n", portsel, pinsel);
    onewire_setup(&owi, &params);
    if (onewire_init(&owi) != ONEWIRE_OK) {
        puts("err: open-drain pin mode not applicable\n");
        owi.pin = GPIO_UNDEF;
        return 1;
    }
    puts("-> OneWire Bus initialized\n");

    /* reset device list */
    memset(_dslist, 0xff, sizeof(_dslist));

    /* search for devices */
    puts("discovering devices on the bus...");
    onewire_rom_t rom;
    unsigned cnt = 0;
    if (onewire_search_first(&owi, &rom) != ONEWIRE_OK) {
        puts("-> err: no devices found!");
        return 1;
    }
    do {
        memcpy(&_dslist[cnt].rom, &rom, sizeof(onewire_rom_t));
        _dslist[cnt].res = dsres;
        ++cnt;
        res = onewire_search_next(&owi, &rom);
    } while (res == ONEWIRE_OK);
    printf("-> found %u devices:\n", cnt);

    for (unsigned i = 0; i < cnt; i++) {
        printf("   [%2i] ROM ADDR: %02x%02x%02x%02x%02x%02x%02x%02x\n", i,
                    (int)rom.u8[0], (int)rom.u8[1],
                    (int)rom.u8[2], (int)rom.u8[3],
                    (int)rom.u8[4], (int)rom.u8[5],
                    (int)rom.u8[6], (int)rom.u8[7]);
    }

    /* initialize all found devices */
    puts("\ninitializing DS18B20 sensors...");
    for (unsigned i = 0; i < cnt; i++) {
        res = ds18b20_init(&_devlist[i], &_dslist[i]);
        if (res != DS18B20_OK) {
            _dslist[i].res = 0xff;  /* mark as invalid */
            printf("-> err: initialization for dev #%u failed\n", i);
        }
        else {
            printf("-> success: dev #%u initialized\n", i);
        }
    }

    return 0;
}

static int _cmd_read(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (unsigned i = 0; i < DEVICE_LIMIT; i++) {
        if (_dslist[i].res != 0xff) {
            char out[20];
            int16_t temp;
            int res = ds18b20_read(&_devlist[i], &temp);
            if (res == DS18B20_OK) {
                size_t pos = fmt_s16_dfp(out, temp, -2);
                out[pos] = '\n';
                printf("dev #%u: %s°C\n", i, out);
            }
            else {
                printf("dev #%u: error reading temperature value\n", i);
            }
        }
    }

    return 0;
}

// static int cmd_test(int argc, char **argv)
// {
//     (void)argc;
//     (void)argv;

//     if (owi.pin == GPIO_UNDEF) {
//         puts("error: onewire bus is not initialized");
//         return 1;
//     }

//     /* generate reset sequence */
//     if (onewire_reset(&owi) == ONEWIRE_NODEV) {
//         puts("no device present on the bus");
//         return 1;
//     }

//     /* read ROM command */
//     onewire_write_byte(&owi, 0x33);

//     uint8_t rom[8];
//     memset(rom, 0, 8);
//     onewire_read(&owi, rom, 8);

//     printf("ROM found is ");
//     for (int i = 0; i < 8; i++) {
//         printf("0x%02x ", (int)rom[i]);
//     }
//     puts("");


//     uint8_t pad[9];
//     memset(pad, 0, 9);

//     onewire_reset(&owi);
//     onewire_write_byte(&owi, 0x55);
//     onewire_write(&owi, rom, 8);

//     onewire_write_byte(&owi, 0xbe);
//     onewire_read(&owi, pad, 9);
//     printf("Scratchpad is ");
//     for (int i = 0; i < 9; i++) {
//         printf("0x%02x ", (int)pad[i]);
//     }
//     puts("");

//     int16_t temp;
//     temp = (pad[1] << 8 | pad[0]);
//     printf("temperature is %i.%i\n", ((int)temp >> 4), ((int)temp & 0xf));

//     return 0;
// }

static const shell_command_t shell_commands[] = {
    { "init", "initialize OneWire interface on given pin", _cmd_init },
    { "read", "get temperature reading from given DS18B20 sensor", _cmd_read },
    // { "test", "foobar", cmd_test },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("1-Wire bus driver test application\n");

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
