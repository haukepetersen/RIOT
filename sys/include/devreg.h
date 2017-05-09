/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_devreg RIOT Device Registry
 * @ingroup     sys
 * @brief       A global registry for handling all kind of devices
 *
 * @{
 *
 * @file
 * @brief       Interface definitions for the global device registry in RIOT
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef DEVREG_H
#define DEVREG_H

#include <stdint.h>
#include <stdbool.h>

#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

device_t *devreg_getnext(device_t *dev);

device_t *devreg_find(device_t *dev, uint32_t mask, uint32_t type);

void devreg_register(device_t *dev);

bool devreg_remove(device_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* DEVREG_H */
/** @} */
