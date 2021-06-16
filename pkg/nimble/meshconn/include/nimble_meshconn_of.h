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

#ifdef __cplusplus
extern "C" {
#endif

// TODO move....
int nimble_meshconn_of_ping_init(void);
int nimble_meshconn_of_hopcnt_init(void);
int nimble_meshconn_of_l2capping_init(void);

enum {
    NIMBLE_MESHCONN_OF_PING,
    NIMBLE_MESHCONN_OF_HOPCNT,
    NIMBLE_MESHCONN_OF_L2CAPPING,
};

typedef void (*nimble_meshconn_of_cb_t)(uint8_t of_type, uint32_t of);

typedef struct {
    list_node_t node;
    nimble_meshconn_of_cb_t cb;
} nimble_meshconn_of_sub_t;

typedef struct nimble_meshconn_of {
    struct nimble_meshconn_of *next;
    uint16_t type;
    int (*activate)(uint8_t role);
    int (*deactivate)(void);
} nimble_meshconn_of_t;

extern list_node_t nimble_meshconn_of_subs;

int nimble_meshconn_of_register(uint8_t of_type, nimble_meshconn_of_t *of_handler);
int nimble_meshconn_of_activate(uint8_t of_type, int role);
int nimble_meshconn_of_deactivate(uint8_t of_type);


void nimble_meshconn_of_notify(uint8_t of_type, uint32_t of);

static inline void nimble_meshconn_of_subscribe(nimble_meshconn_of_sub_t *sub)
{
    assert(sub);
    list_add(&nimble_meshconn_of_subs, &sub->node);
}

static inline int nimble_meshconn_of_unsubscribe(nimble_meshconn_of_sub_t *sub)
{
    assert(sub);
    return (list_remove(&nimble_meshconn_of_subs, &sub->node)) ? 0 : -ENOENT;
}



#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_MESHCONN_OF_H */
/** @} */
