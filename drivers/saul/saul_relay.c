/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_saul
 * @{
 *
 * @file
 * @brief       SAUL relay functions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "saul.h"

static int saul_relay_read(void *dev, phydat_t *res)
{
    saul_reg_t *d = (saul_reg_t *)dev;

    return d->driver->read(d->dev, res);
}

static int saul_relay_write(void *dev, phydat_t *data)
{
    saul_reg_t *d = (saul_reg_t *)dev;

    return d->driver->write(d->dev, data);
}
