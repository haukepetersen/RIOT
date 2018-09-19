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

#include "xtimer.h"
#include "dfplayer.h"
#include "dfplayer_params.h"

static dfplayer_t _dev;

int main(void)
{
    puts("DFPlayer device driver test\n");

    printf("uart %i\n", (int)DFPLAYER_PARAM_UART);

    /* initialize the device */
    if (dfplayer_init(&_dev, dfplayer_params) != DFPLAYER_OK) {
        puts("error: unable to intialize device\n");
        return 1;
    }

    puts("now waking device");
    dfplayer_wakeup(&_dev);

    /* play 10 random songs for 1 sec each */
    /* TODO */
    dfplayer_playback(&_dev);

    return 0;
}
