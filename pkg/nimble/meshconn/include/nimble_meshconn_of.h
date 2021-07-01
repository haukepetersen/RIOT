/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble_meshconn
 *
 * @{
 *
 *
 * OF INPUT:
 * - [opt] OF of each upstream connection
 * -
 *
 * OF OUTPUT:
 * - OF and OF_TYPE, simple as that :-)
 *
 *
 * @file
 * @brief       Meshconn objective function abstraction
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NIMBLE_MESHCONN_OF_H
#define NIMBLE_MESHCONN_OF_H

#include <stdint.h>
#include <stdbool.h>

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NIMBLE_MESHCONN_OF_UNDEF        0

// TODO move....
int nimble_meshconn_of_ping_init(void);
int nimble_meshconn_of_hopcnt_init(void);
int nimble_meshconn_of_l2capping_init(void);

enum {
    NIMBLE_MESHCONN_OF_NONE = 0,
    NIMBLE_MESHCONN_OF_PING = 1,
    NIMBLE_MESHCONN_OF_HOPCNT = 2,
    NIMBLE_MESHCONN_OF_L2CAPPING = 3,
};

typedef void (*nimble_meshconn_of_cb_t)(uint8_t of_type, uint32_t of);

typedef struct {
    list_node_t node;
    nimble_meshconn_of_cb_t cb;
} nimble_meshconn_of_sub_t;

typedef struct nimble_meshconn_of {
    struct nimble_meshconn_of *next;
    uint8_t type;
    int (*activate)(uint8_t role);
    int (*deactivate)(void);
    uint32_t (*get_of)(void);
} nimble_meshconn_of_t;

int nimble_meshconn_of_register(uint8_t of_type, nimble_meshconn_of_t *of_handler);
int nimble_meshconn_of_activate(uint8_t of_type, int role);
int nimble_meshconn_of_deactivate(void);

bool nimble_meshconn_of_is_supported(uint8_t of_type);

uint32_t nimble_meshconn_of_get(void);
uint8_t nimble_meshconn_of_get_type(void);


#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_MESHCONN_OF_H */
/** @} */
