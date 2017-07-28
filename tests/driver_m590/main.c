/*
 * Copyright (C) 2017 Freie Universität Berlin
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
 * @brief       Manual test application for sending text messages using neoway
 *              M590 modems
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "shell.h"
#include "m590.h"
#include "m590_params.h"

static m590_t modem;

static int cmd_text(int argc, char **argv)
{
    if (argc != 3) {
        printf("usage: %s <phone number> <text>\n", argv[0]);
        return 1;
    }

    int res = m590_send_text(&modem, argv[1], argv[2]);
    if (res == M590_OK) {
        puts("Sending text message succeeded:");
        printf("Receiver: %s\n", argv[1]);
        printf("    Text: %s\n", argv[2]);
    }
    else {
        puts("Error: sending text message failed\n");
    }

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "text", "send a text message to the given number", cmd_text },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("M590 GSM modem manual test application\n");
    puts("Initializing M590 modem...");
    int res = m590_init_textmode(&modem, &m590_params[0]);
    if (res == M590_OK) {
        puts("[OK]\n");
    }
    else {
        printf("[FAILED]\n\nreason: ");
        switch (res) {
            case M590_ERR:      puts("unable to initialize AT driver"); break;
            case M590_NODEV:    puts("no M590 modem connected");        break;
            case M590_NOSIM:    puts("no SIM card inserted");           break;
            default:            puts("unknown error code");             break;
        }
        return -1;
    }

    puts("Starting shell now, use 'text' shell command for sending text messages...");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, sizeof(line_buf));

    return 0;
}
