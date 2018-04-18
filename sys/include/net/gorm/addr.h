/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_addr Gorm's Address Handling
 * @ingroup     net_gorm
 * @brief       Gorm's handling of device addresses
 * @{
 *
 * @file
 * @brief       Gorm's address handling API
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_ADDR_H
#define NET_GORM_ADDR_H

#include <stdint.h>

#include "net/ble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Generate a random public device address (using the LUID module)
 *
 * @param[out] buf      write address to this buffer, MUST hold 6 bytes
 */
void gorm_addr_gen_random(uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* NET_GORM_ADDR_H */
/** @} */
