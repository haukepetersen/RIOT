/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_saul
 * @ingroup     drivers_bmp180
 * @{
 *
 * @file
 * @brief       SAUL interface for BMP180 devices
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef SAUL_BMP180_H
#define SAUL_BMP180_H

#include "bmp180.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    saul_t temp;
    saul_t press;
    bmp180_t dev;
} saul_bmp180_t;

typedef struct {
    const char *name_temp;
    const char *name_press;
    const bmp180_params_t *devconf;
} saul_bmp180_params_t;

int saul_bmp180_init(saul_bmp180_t *saul_dev, saul_bmp180_params_t *params);

#ifdef __cplusplus
}
#endif

#endif /* SAUL_BMP180_H */
/** @} */
