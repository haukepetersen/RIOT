/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Application for testing low-level SPI driver implementations
 *
 * This implementation covers both, master and slave configurations.
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shell.h"
#include "periph/spi.h"

#define BUF_SIZE                (512U)

static uint8_t buf[BUF_SIZE];

static struct {
    spi_t dev;
    spi_mode_t mode;
    spi_clk_t clk;
    spi_cs_t cs;
} spiconf;

void print_bytes(char* title, uint8_t* data, size_t len)
{
    printf("%4s", title);
    for (size_t i = 0; i < len; i++) {
        printf("  %2i ", (int)i);
    }
    printf("\n    ");
    for (size_t i = 0; i < len; i++) {
        printf(" 0x%02x", (int)data[i]);
    }
    printf("\n    ");
    for (size_t i = 0; i < len; i++) {
        if (data[i] < ' ' || data[i] > '~') {
            printf("  ?? ");
        }
        else {
            printf("   %c ", (char)data[i]);
        }
    }
    printf("\n\n");
}

int cmd_config(int argc, char **argv)
{
    int tmp, ump;

    if (argc < 5) {
        printf("usage: %s <dev> <mode> <clk> <cs port> <cs pin>\n", argv[0]);
        puts("\tdev:");
        for (int i = 0; i < (int)SPI_NUMOF; i++) {
            printf("\t\t%i: SPI_DEV(%i)\n", i, i);
        }
        puts("\tmode:");
        puts("\t\t0: POL:0, PHASE:0 - on first rising edge");
        puts("\t\t1: POL:0, PHASE:1 - on second rising edge");
        puts("\t\t2: POL:1, PHASE:0 - on first falling edge");
        puts("\t\t3: POL:1, PHASE:1 - on second falling edge");
        puts("\tclk:");
        puts("\t\t0: 100 KHz");
        puts("\t\t1: 400 KHz");
        puts("\t\t2: 1 MHz");
        puts("\t\t3: 5 MHz");
        puts("\t\t4: 10 MHz");
        puts("\tcs port:");
        puts("\t\tPort of the CS pin, set to -1 for hardware chip select");
        puts("\t\tcs pin:");
        puts("\t\tPin used for chip select. If hardware chip select is enabled,\n"
             "\t\tthis value specifies the internal HWCS line");
        return 1;
    }

    /* parse the given SPI device */
    tmp = atoi(argv[1]);
    if (tmp < 0 || tmp >= SPI_NUMOF) {
        puts("error: invalid SPI device specified");
        return 1;
    }
    spiconf.dev = SPI_DEV(tmp);

    /* parse the SPI mode */
    tmp = atoi(argv[2]);
    switch (tmp) {
        case 0: spiconf.mode = SPI_MODE_0; break;
        case 1: spiconf.mode = SPI_MODE_1; break;
        case 2: spiconf.mode = SPI_MODE_2; break;
        case 3: spiconf.mode = SPI_MODE_3; break;
        default:
            puts("error: invalid SPI mode specified");
            return 1;
    }

    /* parse the targeted clock speed */
    tmp = atoi(argv[3]);
    switch (tmp) {
        case 0: spiconf.clk = SPI_CLK_100KHZ; break;
        case 1: spiconf.clk = SPI_CLK_400KHZ; break;
        case 2: spiconf.clk = SPI_CLK_1MHZ;   break;
        case 3: spiconf.clk = SPI_CLK_5MHZ;   break;
        case 4: spiconf.clk = SPI_CLK_10MHZ;  break;
        default:
            puts("error: invalid bus speed specified");
            return 1;
    }

    /* parse chip select port and pin */
    tmp = atoi(argv[4]);
    ump = atoi(argv[5]);
    if (ump < 0 || tmp < -1) {
        puts("error: invalid CS port/pin combination specified");
    }
    if (tmp == -1) {                    /* hardware chip select line */
        spiconf.cs = SPI_HWCS(ump);
    }
    else {
        spiconf.cs = (spi_cs_t)GPIO_PIN(tmp, ump);
    }

    /* test setup */
    tmp = spi_init_cs(spiconf.dev, spiconf.cs);
    if (tmp != SPI_OK) {
        puts("error: unable to initialize the given chip select line");
        return 1;
    }
    tmp = spi_acquire(spiconf.dev, spiconf.cs, spiconf.mode, spiconf.clk);
    if (tmp == SPI_NOMODE) {
        puts("error: given SPI mode is not supported");
        return 1;
    }
    else if (tmp == SPI_NOCLK) {
        puts("error: targeted clock speed is not supported");
        return 1;
    }
    else if (tmp != SPI_OK) {
        puts("error: unable to acquire bus with given parameters");
        return 1;
    }
    spi_release(spiconf.dev);

    return 0;
}

int cmd_transfer(int argc, char **argv)
{
    size_t len;

    if (argc < 2) {
        printf("usage: %s <data>\n", argv[0]);
    }

    if (spiconf.dev == SPI_UNDEF) {
        puts("error: SPI is not initialized, please initialize bus first");
        return 1;
    }

    /* get bus access */
    if (spi_acquire(spiconf.dev, spiconf.mode,
                    spiconf.clk, spiconf.cs) != SPI_OK) {
        puts("error: unable to acquire the SPI bus");
        return 1;
    }

    /* transfer data */
    len = strlen(argv[1]);
    spi_transfer_bytes(spiconf.dev, spiconf.cs, false, argv[2], buf, len);

    /* release the bus */
    spi_release(spiconf.dev);

    /* print results */
    print_bytes("Sent bytes", (uint8_t *)argv[2], len);
    print_bytes("Received bytes", buf, len);

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "config", "Setup a particular SPI configuration", cmd_config },
    { "send", "Transfer string to slave", cmd_transfer },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("Manual SPI peripheral driver test");
    puts("Refer to the README.md file for more information.\n");

    printf("There are %i SPI devices configured for your platform:\n",
           (int)SPI_NUMOF);

    /* reset local SPI configuration */
    spiconf.dev = SPI_UNDEF;

    /* do base initialization of configured SPI devices
       TODO: remove once spi_init calls are moved into board_init/kernel_init */
    for (int i = 0; i < (int)SPI_NUMOF; i++) {
        spi_init(SPI_DEV(i));
    }

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
