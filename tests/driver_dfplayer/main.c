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

static int _cmd_cur(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int res = dfplayer_current_track(&_dev);
    if (res < DFPLAYER_OK) {
        puts("Error: unable to read current track");
    }
    else {
        printf("Current track: %i\n", res);
    }
    return 0;
}

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
        printf("Volume: %i\n", dfplayer_vol_get(&_dev));
        printf(" usage: %s <up|down|[0-31]>\n", argv[0]);
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

static const char *_eq_settings[] = {
    "normal", "pop", "rock", "jazz", "classic", "base",
};

static int _cmd_eq(int argc, char **argv)
{
    if (argc < 2) {
        int eq = dfplayer_eq_get(&_dev);
        if (eq >= DFPLAYER_OK) {
            printf("EQ setting: %s\n", _eq_settings[eq]);
        }
        else {
            puts("Unable to read current EQ setting");
        }
        printf("Usage: %s [normal|pop|rock|jazz|classic|base]\n", argv[0]);
        return 1;
    }

    for (unsigned i = 0; i < 6; i++) {
        if (strcmp(_eq_settings[i], argv[1]) == 0) {
            dfplayer_eq_set(&_dev, (dfplayer_eq_t)i);
            break;
        }
    }

    return 0;
}

static const char *_modes[] = {
    "repeat", "repeat_f", "repeat_s", "random",
};

static int _cmd_mode(int argc, char **argv)
{
    if (argc < 2) {
        int mode = dfplayer_mode_get(&_dev);
        if (mode >= DFPLAYER_OK) {
            printf("Playback mode: %s\n", _modes[mode]);
        }
        else {
            puts("Unable to read current playback mode");
        }
        printf("Usage: %s [repeat|repeat_f|repeat_s|random]", argv[0]);
        return 1;
    }

    for (unsigned i = 0; i < 4; i++) {
        if (strcmp(_modes[i], argv[1]) == 0) {
            dfplayer_mode_set(&_dev, (dfplayer_mode_t)i);
            break;
        }
    }

    return 0;
}

static int _cmd_reset(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    if (dfplayer_reset(&_dev) == DFPLAYER_OK) {
        puts("Device reset ok");
    }
    else {
        puts("Error while triggering device reset");
    }
    return 0;
}

static int _cmd_sleep(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <0|1>\n", argv[0]);
        return 1;
    }
    switch (atoi(argv[1])) {
        case 0:
            dfplayer_wakeup(&_dev);
            puts("Put device do normal working mode");
            break;
        case 1:
            dfplayer_standby(&_dev);
            puts("Put device into sleep mode");
            break;
        default:
            puts("Error: unable to parse parameter");
    }
    return 0;
}

static int _cmd_ver(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int res = dfplayer_ver(&_dev);
    if (res < 0) {
        puts("Error: unable to read version");
    }
    else {
        printf("Device version is: %i\n", res);
    }
    return 0;
}

static int _cmd_status(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int res = dfplayer_status(&_dev);
    if (res < DFPLAYER_OK) {
        puts("Error: unable to read device status\n");
    }
    else {
        printf("Device Status: %i\n", res);
    }
    return 0;
}

static int _cmd_count(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int res = dfplayer_count_files(&_dev);
    if (res < DFPLAYER_OK) {
        puts("Error: unable to count number of files on TF card");
    }
    else {
        printf("Number of files on TF card: %i\n", res);
    }
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "cur", "get number of current track", _cmd_cur },
    { "play", "play the current track", _cmd_play },
    { "pause", "pause playpack", _cmd_pause },
    { "next", "play next track", _cmd_next },
    { "prev", "play previous track", _cmd_prev },
    { "vol", "volume control", _cmd_vol },
    { "eq", "equalizer control", _cmd_eq },
    { "mode", "playback mode control", _cmd_mode },
    { "reset", "reset the device", _cmd_reset },
    { "sleep", "put device to sleep", _cmd_sleep },
    { "ver", "get device version", _cmd_ver },
    { "status", "get current device status", _cmd_status },
    { "count", "count the number of available tracks", _cmd_count },
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
