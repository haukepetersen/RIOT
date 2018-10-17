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

#ifndef RDEV_H
#define RDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/**> TODO: find sensible hierarchical names */
typedef union {
    uint32_t full,
    struct {
        /* TODO: possibly use different partition of descriptive space */
        uint8_t l0;     /* e.g. network, sensor, actuator, ... */
        uint8_t l1;     /* e.g. wired, wireless, ..., temp, acc, imu-agm */
        uint8_t l2;     /* e.g. ethernet, 15.4, ble, ...*/
        uint8_t l3;     /* e.g. at86rf, nrfble, bmx080 */
    } u8,
} rdev_type_t;

/* generic, static device interface, also contains device type descriptor */
typedef struct {
    rdev_type_t;
    int (*init)(void *dev, const void *params);
    int (*cmd)(void *dev, devopt_t opt, uint32_t ctx, void *val, size_t len);
} rdev_base_t;

typedef struct rdev {
#ifdef MODULE_DEVREG
    struct rdev *next;
#endif
#ifdef MODULE_DEVREG_NAME
    const char *name;
#endif
    const rdev_base_t *driver;
} rdev_t;


int rdev_register(rdev_t *rdev);
int rdev_unregister(rdev_t *rdev);


static inline uint32_t rdev_get_type(const rdev_t *dev);
{
    return dev->type.full;
}

/* more convenience functions to come... */

#ifdef __cplusplus
}
#endif

#endif /* RDEV_H */
/** @} */
