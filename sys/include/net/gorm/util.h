/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_util Gorm's Helpers
 * @ingroup     net_gorm
 * @brief       Utility and helper functions for Gorm
 * @{
 *
 * @file
 * @brief       Gorm's little helpers
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_UTIL_H
#define NET_GORM_UTIL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Read 16-bit little endian var from buffer into uint16 variable
 *
 * This function can be used in any context, as it does not have any
 * requirements on the alignment of @p buf.
 *
 * @param[in] buf       read from this buffer, must be of size 2
 *
 * @return  16-bit unsigned integer in host byte order
 */
static inline uint16_t gorm_util_letohs(const uint8_t *buf)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (uint16_t)((buf[1] << 8) | buf[0]);
#else
    return (uint16_t)((buf[0] << 8) | buf[1]);
#endif
}

/**
 * @brief   Write given uint16 variable into buffer in little endian byte order
 *
 * This function can be used in any context, as it does not have any
 * requirements on the alignment of @p buf.
 *
 * @param[out] buf      write to this buffer, must be able to hold 2 bytes
 * @param[in]  val      value to write to buffer
 */
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

#ifdef __cplusplus
}
#endif

#endif /* NET_GORM_UTIL_H */
/** @} */
