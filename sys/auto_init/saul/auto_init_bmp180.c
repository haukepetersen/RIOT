/*
 * Copyright (C) 2016 Inria
 *               2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     auto_init_saul
 * @{
 *
 * @file
 * @brief       Auto initialization of BMP180 driver.
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "log.h"
#include "saul_reg.h"
#include "saul/saul_bmp180.h"
#include "bmp180_params.h"

/**
 * @brief   Define the number of configured sensors
 */
#define NUMOF   (sizeof(saul_bmp180_params) / sizeof(saul_bmp180_params[0]))

/**
 * @brief   Allocation of memory for device descriptors
 */
static saul_bmp180_t devmem[NUMOF];

void auto_init_bmp180(void)
{
    for (unsigned i = 0; i < NUMOF; i++) {
        if (saul_bmp180_init(&devmem[i], &saul_bmp180_params[i]) != BMP180_OK) {
            LOG_ERROR("[auto_init_saul] error initializing bmp180 #%u\n", i);
            continue;
        }
        saul_reg_add(devmem[i].temp);
    }
}
