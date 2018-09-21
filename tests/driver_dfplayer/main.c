/*
 * Copyright (C) 2018 Hauke Petersen <devel@haukepetersen.de>
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
 * @brief       Test application for the DFPLayer device driver
 *
 * @author      Hauke Petersen <devel@haukepetersen.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "dfplayer.h"
#include "dfplayer_params.h"

static dfplayer_t _dev;

static int _cmd_play(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    dfplayer_play(&_dev);
    return 0;
}

static int _cmd_pause(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    dfplayer_pause(&_dev);
    return 0;
}

static int _cmd_next(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    dfplayer_next(&_dev);
    return 0;
}

static int _cmd_prev(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    dfplayer_prev(&_dev);
    return 0;
}

static int _cmd_vol(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <up|down|[0-31]>\n", argv[0]);
        return 1;
    }
    if (strncmp(argv[1], "up", 2) == 0) {
        dfplayer_vol_up(&_dev);
        puts("Volume UP");
    }
    else if (strncmp(argv[1], "down", 4) == 0) {
        dfplayer_vol_down(&_dev);
        puts("Volume: DOWN");
    }
    else {
        int vol = atoi(argv[1]);
        if ((vol < (int)DFPLAYER_VOL_MIN) || (vol > (int)DFPLAYER_VOL_MAX)) {
            puts("error: volume level not applicable");
        }
        else {
            dfplayer_vol_set(&_dev, (uint16_t)vol);
            printf("Volume: set to %i\n", vol);
        }
    }

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "play", "play the current track", _cmd_play },
    { "pause", "pause playpack", _cmd_pause },
    { "next", "play next track", _cmd_next },
    { "prev", "play previous track", _cmd_prev },
    { "vol", "set the volume", _cmd_vol },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("DFPlayer device driver test\n");

    printf("uart %i\n", (int)DFPLAYER_PARAM_UART);

    /* initialize the device */
    if (dfplayer_init(&_dev, dfplayer_params) != DFPLAYER_OK) {
        puts("error: unable to intialize device\n");
        return 1;
    }

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
