/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_adproc
 * @ingroup     sys
 * @brief       Generic BT advertising and scan request data processor
 * @{
 *
 * @file
 * @brief       Interface for the generic BT AD data processing module
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef ADPROC_H
#define ADPROC_H

#include <stdint.h>
#include <stddef.h>

#include "net/ble.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ADPROC_OK       =  0,
    ADPROC_NOTFOUND = -1,
};

typedef struct {
    uint8_t *data;
    uint8_t len;
} adproc_field_t;

int adproc_find(uint8_t type, adproc_field_t *field,
                uint8_t *buf, size_t buf_len);

int adproc_get_str(uint8_t type, char *res, uint8_t *buf, size_t buf_len);

#ifdef __cplusplus
}
#endif

#endif /* ADPROC_H */
/** @} */
