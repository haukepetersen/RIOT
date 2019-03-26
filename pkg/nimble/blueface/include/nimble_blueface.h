/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble
 * @{
 *
 * @file
 * @brief       NimBLE CCN-lite face adapter
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NIMBLE_BLUEFACE_H
#define NIMBLE_BLUEFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#include "host/ble_gap.h"
#include "host/ble_l2cap.h"

typedef struct nimble_blueface nimble_blueface_t;

typedef void(*nimble_blueface_evt_cb_t)(nimble_blueface_t *bf, unsigned event);
typedef void(*nimble_blueface_data_cb_t)(nimble_blueface_t *bf,
                                         void * data, size_t len);


struct nimble_blueface {
    struct ble_l2cap_chan *chan;        /**< L2CAP channel identifier */
    nimble_blueface_evt_cb_t evt_cb;
    nimble_blueface_data_cb_t data_cb;
};


int nimble_blueface_init(void);

int nimble_blueface_connect(nimble_blueface_t *bf,
                            ble_addr_t *addr, uint32_t max_wait,
                            nimble_blueface_evt_cb_t on_evt,
                            nimble_blueface_data_cb_t on_data,
                            void *arg);

int nimble_blueface_terminate(nimble_blueface_t *bf);

/**
 * - allocate mbuf
 * - copy data into mbuf
 * - hand mbuf to NimBLE
 */
int nimble_blueface_send(nimble_blueface_t *bf, void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_BLUEFACE_H */
/** @} */
