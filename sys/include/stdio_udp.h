/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_stdio_udp Standard I/O over UDP
 * @ingroup     sys_stdio
 * @brief       Standard input/output over UDP
 * @{
 *
 * @file
 * @brief       UDP mapped standard I/O interface definition
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef STDIO_UDP_H
#define STDIO_UDP_H

#include "riot_stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STDIO_UDP_PORT
/**
 * @brief   UDP port used to listen for standard input
 */
#define STDIO_UDP_PORT          (2305)
#endif

/**
 * @brief   We need a 'second phase' initialization, to be called after the
 *          underlying network stack was initialized
 */
void stdio_udp_init(void);

#ifdef __cplusplus
}
#endif

#endif /* STDIO_H */
/** @} */
