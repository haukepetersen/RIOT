/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_driver Generic base device
 * @ingroup     drivers
 * @brief       Generic base device as parent for all devices in RIOT
 *
 * @{
 *
 * @file
 * @brief       RIOT base device definition
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include "devopt.h"
#include "device_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEVELHELP
#define DEVICE_NAME(x)          .name = x,
#else
#define DEVICE_NAME(x)
#endif

typedef struct device device_t;

typedef struct {
    uint32_t type;
    int (*init)(device_t *dev);
    int (*set)(device_t *dev, devopt_t opt, void *arg, size_t len);
    int (*get)(device_t *dev, devopt_t opt, void *arg, size_t maxlen);
} device_driver_t;

struct device {
    struct device *next;
    const device_driver_t *driver;
#ifdef DEVELHELP
    const char *name;
#endif
};

static inline bool device_is_fam(device_t *dev, uint32_t fam)
{
    return ((dev->driver->type & DEVICE_FAM_MASK) == fam);
}

static inline bool device_is_group(device_t *dev, uint32_t type)
{
    return ((dev->driver->type & DEVICE_GROUP_MASK) == type);
}

static inline bool device_is_model(device_t *dev, uint32_t model)
{
    return ((dev->driver->type & DEVICE_MODEL_MASK) == model);
}

static inline uint8_t device_get_fam(device_t *dev)
{
    return (uint8_t)((dev->driver->type & DEVICE_FAM_MASK) >> 24);
}

static inline uint8_t device_get_group(device_t *dev)
{
    return (uint8_t)((dev->driver->type & DEVICE_GROUP_MASK) >> 16);
}

static inline uint16_t device_get_model(device_t *dev)
{
    return (uint16_t)(dev->driver->type & DEVICE_MODEL_MASK);
}

static inline int device_init(device_t *dev)
{
    return dev->driver->init(dev);
}

static inline int device_get(device_t *dev, devopt_t opt, void *val, size_t maxlen)
{
    return dev->driver->get(dev, opt, val, maxlen);
}

static inline int device_set(device_t *dev, devopt_t opt, void *val, size_t len)
{
    return dev->driver->set(dev, opt, val, len);
}

int device_noinit(device_t *dev)
{
    (void)dev;
    return -ENOTSUP;
}

int device_nogetset(device_t *dev, devopt_t opt, void *val, size_t len)
{
    (void)dev;
    (void)opt;
    (void)val;
    (void)len;
    return -ENOTSUP;
}

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H */
/** @} */
