/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_devreg
 * @{
 *
 * @file
 * @brief       RIOT global device registration module
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "irq.h"
#include "assert.h"
#include "device.h"

static device_t *devlist = NULL;

static inline device_t *find_before(device_t *dev)
{
    for (device_t *tmp = devlist; (tmp && tmp->next); tmp = tmp->next) {
        if (tmp->next == dev) {
            return tmp;
        }
    }
    return NULL;
}

device_t *devreg_getnext(device_t *dev)
{
    if (dev) {
        return dev->next;
    }
    else {
        return devlist;
    }
}

device_t *devreg_find(device_t *dev, uint32_t mask, uint32_t type)
{
    for (device_t *tmp = (dev) ? dev->next: devlist; tmp; tmp = tmp->next) {
        if ((tmp->driver->type & mask) == type) {
            return tmp;
        }
    }
    return NULL;
}

void devreg_register(device_t *dev)
{
    assert(dev && dev->next == NULL);

    unsigned is = irq_disable();
    dev->next = devlist;
    devlist = dev;
    irq_restore(is);
}

bool devreg_remove(device_t *dev)
{
    assert(dev);

    bool res = true;
    unsigned is = irq_disable();
    device_t *before = find_before(dev);
    if (before) {
        before->next = dev->next;
        dev->next = NULL;
    }
    else {
        res = false;
    }
    irq_restore(is);

    return res;
}
