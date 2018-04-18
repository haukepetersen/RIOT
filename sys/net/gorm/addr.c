/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm_addr
 * @{
 *
 * @file
 * @brief       Implementation of Gorm's address handling
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "assert.h"
#include "luid.h"

#include "net/gorm/addr.h"

/* TODO: this code is shared with Skald, so factor out into some generic BLE
 *       address code */
void gorm_addr_gen_random(uint8_t *buf)
{
    assert(buf);

    luid_get(buf, BLE_ADDR_LEN);
    /* swap byte 0 and 5, so that the unique byte given by luid does not clash
     * with universal/local and individual/group bits of address */
    uint8_t tmp = buf[5];
    buf[5] = buf[0];
    /* make address individual and local */
    buf[0] = ((tmp & 0xfc) | 0x02);
}
