/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_thingy52
 * @{
 *
 * @file
 * @brief       Board initialization for the Thingy:52
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "cpu.h"
#include "board.h"
// #include "sx150x.h"
// #include "sx150x_params.h"

// sx150x_t thingy52_sx;

void board_init(void)
{
    /* initialize the CPU */
    cpu_init();
}
