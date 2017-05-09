/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_driver
 *
 * @{
 *
 * @file
 * @brief       Driver device type definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef DEVOPT_H
#define DEVOPT_H

#ifdef __cplusplus
extern "C" {
#endif




typedef enum {
    DEVOPT_CMD,
    DEVOPT_POWER_MODE
} devopt_t;

#ifdef __cplusplus
}
#endif

#endif /* DEVOPT_H */
/** @} */
