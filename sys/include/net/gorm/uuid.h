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
 * @brief       Gorm's UUID handling
 *
 * # Implementation restrictions
 * - only support for 128-bit and 16-bit UUIDs as of now
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_UUID_H
#define NET_GORM_UUID_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief   Static UUID initializer
 */
#define GORM_UUID(u16, b)       { .uuid16 = u16, .base = b }

/**
 * @brief   Structure for storing the base of 128-bit UUIDs
 */
typedef union {
    uint8_t raw[16];        /**< byte-wise access */
    uint16_t u16[8];        /**< 16-bit access */
    uint32_t u32[4];        /**< 32-bit access */
} gorm_uuid_base_t;

/**
 * @brief   Structure for storing UUIDs
 */
typedef struct {
    const gorm_uuid_base_t *base;   /**< base, set to NULL for 16-bit UUIDs */
    uint16_t uuid16;                /**< 16-bit part of UUID */
} gorm_uuid_t;

/**
 * @brief   Read a UUID from the given raw buffer
 *
 * @param[out] uuid     parse UUID into this buffer
 * @param[in]  buf      read from this buffer
 * @param[in]  len      length of UUID in @p buf, must be 2 or 16
 */
void gorm_uuid_from_buf(gorm_uuid_t *uuid, const uint8_t *buf, size_t len);

/**
 * @brief   Write UUID into the given buffer
 *
 * @param[out] buf      write raw UUID into this buffer, must be able to hold up
 *                      to 16 byte, depending on the length of @p uuid (2 or 16)
 * @param[in]  uuid     UUID to write to @p buf
 *
 * @return  2 if @p uuid is a 16-bit UUID
 * @return  16 if @p uuid is a 128-bit UUID
 */
size_t gorm_uuid_to_buf(uint8_t *buf, const gorm_uuid_t *uuid);

/**
 * @brief   Compare two given UUIDs
 *
 * @param[in] a         first UUID to compare
 * @param[in] b         second UUID to compare
 *
 * @return  0 if UUIDs are different
 * @return  != 0 if UUIDs are equal
 */
int gorm_uuid_equal(const gorm_uuid_t *a, const gorm_uuid_t *b);

/**
 * @brief   Print the given UUID to STDOUT
 *
 * @param[in] uuid      UUID to print
 */
void gorm_uuid_print(const gorm_uuid_t *uuid);

/**
 * @brief   Test if the given UUID is a BT SIG UUID (a 16-bit UUID)
 *
 * @param[in] uuid      UUID to test
 *
 * @return  0 if UUID is not a SIG UUID (not a 16-bit UUID)
 * @return  != 0 if UUID is a SIG UUID (a 16-bit UUID)
 */
static inline int gorm_uuid_sig(const gorm_uuid_t *uuid)
{
    return (uuid->base == NULL);
}

/**
 * @brief   Test if the given number matches the 16-bit part of the given UUID
 *
 * @param[in] uuid      UUID to test
 * @param[in] uuid16    16-bit value to check for
 *
 * @return  0 if 16-bit part of UUID does not match
 * @return  != 0 if 16-bit part of UUID matches
 */
static inline int gorm_uuid_eq16(const gorm_uuid_t *uuid, uint16_t uuid16)
{
    return (gorm_uuid_sig(uuid) && (uuid->uuid16 == uuid16));
}

/**
 * @brief   Get the length of the given UUID
 *
 * @param[in] uuid      UUID to test
 *
 * @return  2 if @p uuid is a 16-bit UUID
 * @return  16 if @p uuid is a 128-bit UUID
 */
static inline size_t gorm_uuid_len(const gorm_uuid_t *uuid)
{
    return (gorm_uuid_sig(uuid)) ? 2 : 16;
}

/**
 * @brief   Initialize the given UUID buffer
 *
 * @param[out] uuid     UUID buffer to initialize
 * @param[in]  base     UUID base, may be NULL for 16-bit UUIDs
 * @param[in]  uuid16   16-bit part of the UUID
 */
static inline void gorm_uuid_init(gorm_uuid_t *uuid,
                                  const gorm_uuid_base_t *base, uint16_t uuid16) {
    uuid->base = base;
    uuid->uuid16 = uuid16;
}

#endif /* NET_GORM_UUID_H */
/** @} */
