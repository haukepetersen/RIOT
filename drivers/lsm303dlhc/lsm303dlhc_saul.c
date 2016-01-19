/*
 * Copyright (C) 2015-2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     driver_lsm303dlhc
 * @{
 *
 * @file
 * @brief       LSM303DLHC adaption to the RIOT actuator/sensor interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <string.h>

#include "saul.h"
#include "lsm303dlhc.h"
#include "lsm303dlhc_params.h"

#ifdef MODULE_SAUL_REG
#define DEV_NUM     (sizeof(lsm303dlhc_params)/sizeof(lsm303dlhc_params[0]))

lsm303dlhc_t lsm303dlhc_devs[DEV_NUM];

int lsm303dlhc_init_saul(void)
{
    for (int i = 0; i < DEV_NUM; i++) {
        if (lsm303dlhc_init(&lsm303dlhc_devs[i], &lsm303dlhc_params[i]) < 0) {
            return -1;
        }
        for (int j = 0; j < LSM303DLHC_SAUL_DEVS; j++) {
            lsm303dlhc_saul[i][j].dev = &lsm303dlhc_devs[i];
            saul_reg_add(&(lsm303dlhc_saul[i][j]));
        }
    }
    return 0;
}
#endif

static int read_acc(void *dev, phydat_t *res)
{
    lsm303dlhc_t *d = (lsm303dlhc_t *)dev;
    lsm303dlhc_read_acc(d, (lsm303dlhc_3d_data_t *)res);
    res->unit = UNIT_G;
    res->scale = 0;
    return 3;
}

static int read_mag(void *dev, phydat_t *res)
{
    lsm303dlhc_t *d = (lsm303dlhc_t *)dev;
    lsm303dlhc_read_mag(d, (lsm303dlhc_3d_data_t *)res);
    res->unit = UNIT_GS;
    res->scale = 0;
    return 3;
}

static int write(void *dev, phydat_t *state)
{
    return -ENOTSUP;
}

const saul_driver_t lsm303dlhc_saul_acc_driver = {
    .read = read_acc,
    .write = write,
    .type = SAUL_SENSE_ACCEL,
};

const saul_driver_t lsm303dlhc_saul_mag_driver = {
    .read = read_mag,
    .write = write,
    .type = SAUL_SENSE_MAG,
};
