/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    TODO
 * @ingroup     drivers
 * @brief       TODO
 *
 * TODO
 *
 * @{
 *
 * @file
 * @brief       TODO
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef SAUL2_H
#define SAUL2_H

#include "rdev.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*saul_event_handler_t)(saul2_dev_t *dev, uint32_t ctx,
                                    phydat2_t *data, size_t len);

/* TODO: work on naming of member functions */
typedef struct {
    int (*read)(saul2_dev_t *dev, uint32_t ctx, phydat2_t *data, size_t len);
    int (*write)(saul2_dev_t *dev, uint32_t ctx, phydat2_t *data, size_t len);
    int (*on_event)(saul2_dev_t *dev, saul_event_t event,
                    saul_event_handler_t handler);
} saul2_driver_t;

typedef struct saul_dev {
    rdev_t rdev;
    const saul2_driver_t *driver;
} saul2_dev_t;

#ifdef __cplusplus
}
#endif

#endif /* SAUL2_H */
/** @} */
