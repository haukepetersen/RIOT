/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
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
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include <string.h>

#include "assert.h"

#include "net/gorm/uuid.h"
#include "net/gorm/util.h"

void gorm_uuid_from_buf(gorm_uuid_t *uuid, uint8_t *buf, size_t len)
{
    assert(uuid && buf && ((len == 2) || (len == 16)));

    if (len == 2) {
        uuid->uuid16 = gorm_util_letohs(buf);
        uuid->base = NULL;
    }
    else {
        uuid->uuid16 = gorm_util_letohs(&buf[2]);
        uuid->base = (gorm_uuid_base_t *)buf;
    }
}

size_t gorm_uuid_to_buf(uint8_t *buf, const gorm_uuid_t *uuid)
{
    assert(buf && uuid);

    if (gorm_uuid_sig(uuid)) {
        memcpy(buf, &uuid->uuid16, 2);
        return 2;
    }
    else {
        memcpy(buf, uuid->base->raw, 2);
        memcpy((buf + 2), &uuid->uuid16, 2);
        memcpy((buf + 4), (uuid->base->raw + 4), 12);
        return 16;
    }
}

int gorm_uuid_cmp(const gorm_uuid_t *a, const gorm_uuid_t *b)
{
    if (a->uuid16 == b->uuid16) {
        if (a->base == NULL) {
            return (b->base == NULL);
        }
        else {
            return ((memcmp(a->base->raw, b->base->raw, 2) == 0) &&
                    (memcmp(&a->base->raw[4], &b->base->raw[4], 12) == 0));
        }

    }
    return 0;
}
