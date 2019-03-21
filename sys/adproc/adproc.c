/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_adproc
 * @{
 *
 * @file
 * @brief       Implementation of the generic BT AD data processing module
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <string.h>

#include "adproc.h"

#define POS_TYPE            (1U)
#define POS_DATA            (2U)

int adproc_find(uint8_t type, adproc_field_t *field,
                uint8_t *buf, size_t buf_len)
{
    unsigned pos = 0;

    while ((pos + 1) < buf_len) {
        uint8_t len = buf[pos];

        if (buf[pos + POS_TYPE] == type) {
            field->data = buf + pos + POS_DATA;
            field->len = len - 1;           /* take away the type field */
            return ADPROC_OK;
        }

        pos += (len + 1);
    }

    return ADPROC_NOTFOUND;
}

int adproc_get_str(uint8_t type, char *str, uint8_t *buf, size_t buf_len)
{
    adproc_field_t f;
    int res = adproc_find(type, &f, buf, buf_len);
    if (res != ADPROC_OK) {
        return res;
    }

    memcpy(str, f.data, f.len);
    str[f.len + 1] = '\0';

    return ADPROC_OK;
}
