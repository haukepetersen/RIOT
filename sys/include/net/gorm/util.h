/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm
 * @{
 *
 * @file
 * @brief       Gorm's little helpers
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_UTIL_H
#define GORM_UTIL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t gorm_util_letohs(uint8_t *buf)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (uint16_t)((buf[1] << 8) | buf[0]);
#else
    return (uint16_t)((buf[0] << 8) | buf[1]);
#endif
}

static inline uint16_t gorm_util_betohs(uint8_t *buf)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (uint16_t)((buf[0] << 8) | buf[1]);
#else
    return (uint16_t)((buf[1] << 8) | buf[0]);
#endif
}

static inline void gorm_util_htoles(uint8_t *buf, uint16_t val)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    buf[0] = (uint8_t)(val & 0xff);
    buf[1] = (uint8_t)(val >> 8);
#else
    buf[0] = (uint8_t)(val >> 8);
    buf[1] = (uint8_t)(val & 0xff);
#endif
}

static inline void gorm_util_htobes(uint8_t *buf, uint16_t val)
{
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

#endif /* GORM_LL_H */
/** @} */
