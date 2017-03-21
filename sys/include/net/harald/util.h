/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    harald_util Harald's Shared Utilities
 * @ingroup     net_harald
 * @brief       Shared utility functions used by Harald
 *
 * @{
 * @file
 * @brief       Harald's utilities interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef HARALD_UTIL_H
#define HARALD_UTIL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t harald_util_get_u16(const void *addr)
{
    const uint8_t *buf = (const uint8_t *)addr;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (uint16_t)((buf[0] << 8) | buf[1]);
#else
    return (uint16_t)((buf[1] << 8) | buf[0]);
#endif
}

static inline void harald_util_set_u16(void *addr, uint16_t val)
{
    uint8_t *buf = (uint8_t *)addr;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    buf[0] = (uint8_t)(val >> 8);
    buf[1] = (uint8_t)(val & 0xff);
#else
    buf[0] = (uint8_t)(val & 0xff);
    buf[1] = (uint8_t)(val >> 8);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* HARALD_UTIL_H */
/** @} */
