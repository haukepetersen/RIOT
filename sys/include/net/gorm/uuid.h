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
 * Implementation restrictions:
 * - only support for 128-bit and 16-bit UUIDs (is that true?)
 * - handle allocation:
 *    - 0b0xxxxxxxNNNNNNNN -> 16-bit UUIDed services
 *    - 0b1xxxxxxxNNNNNNNN -> 128-bit UUIDed services
 *    - 0bxxxxxxxxCCCC0000 -> 4 MSB of lower byte: characteristics
 *    - 0bxxxxxxxxCCCCDDDD -> 4 LSB of lower byte: char descriptions
 *
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_UUID_H
#define GORM_UUID_H

#include <stdint.h>
#include <stddef.h>

#define GORM_UUID(u16, b)       { .uuid16 = u16, .base = b }

/* TODO: move to ble.h */
#define GORM_UUID_GAP               (0x1800)
#define GORM_UUID_ATT               (0x1801)

/* TODO: move to ble.h */
#define GORM_UUID_DEVICE_NAME       (0x2a00)
#define GORM_UUID_APPEARANCE        (0x2a01)
#define GORM_UUID_PREF_CON_PARAM    (0x2a04)

typedef union {
    uint8_t raw[16];
    uint16_t u16[8];
    uint32_t u32[4];
} gorm_uuid_base_t;

typedef struct {
    const gorm_uuid_base_t *base;   /**< base UUID, NULL refers to Bluetooth UUID */
    uint16_t uuid16;
} gorm_uuid_t;


void gorm_uuid_from_buf(gorm_uuid_t *uuid, uint8_t *buf, size_t len);
size_t gorm_uuid_to_buf(uint8_t *buf, const gorm_uuid_t *uuid);

/**
 * @brief   Compare two given UUIDs
 *
 * @param[in] a     first UUID to compare
 * @param[in] b     second UUID to compare
 *
 * @return  0 if UUIDs are different
 * @return  != 0 if UUIDs are equal
 */
int gorm_uuid_equal(const gorm_uuid_t *a, const gorm_uuid_t *b);


static inline int gorm_uuid_sig(const gorm_uuid_t *uuid)
{
    return (uuid->base == NULL);
}

static inline int gorm_uuid_eq16(const gorm_uuid_t *uuid, uint16_t uuid16)
{
    return (gorm_uuid_sig(uuid) && (uuid->uuid16 == uuid16));
}

static inline size_t gorm_uuid_len(const gorm_uuid_t *uuid)
{
    return (gorm_uuid_sig(uuid)) ? 2 : 16;
}

static inline void gorm_uuid_init(gorm_uuid_t *uuid, gorm_uuid_base_t *base,
                                  uint16_t uuid16) {
    uuid->base = base;
    uuid->uuid16 = uuid16;
}



#endif /* GORM_UUID_H */
