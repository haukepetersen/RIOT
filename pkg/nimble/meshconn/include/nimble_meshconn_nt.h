/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble_meshconn_nt
 *
 * @{
 *
 * @file
 * @brief       Meshconn neighbor table
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NIMBLE_MESHCONN_NT_H
#define NIMBLE_MESHCONN_NT_H

#include <stdint.h>

#include "list.h"

#include "host/ble_hs.h"

#ifdef __cplusplus
extern "C" {
#endif



#define NIMBLE_MESHCONN_NT_TABLE_SIZE               10
// #define NIMBLE_MESHCONN_NT_TIMEOUT_MS               30000
#define NIMBLE_MESHCONN_NT_TIMEOUT_MS               7500 // DEBUG

#define NIMBLE_MESHCONN_NT_TIMEOUT_REEVEAL          (50 * NIMBLE_MESHCONN_NT_TIMEOUT_MS) // TODO

typedef enum {
    NIMBLE_MESHCONN_NT_NOTCONN = 0,
    NIMBLE_MESHCONN_NT_UPSTREAM,
    NIMBLE_MESHCONN_NT_DOWNSTREAM,
    // NIMBLE_MESHCONN_NT_LOCKED,
    NIMBLE_MESHCONN_NT_UNUSED,
} nimble_meshconn_nt_state_t;

typedef struct {
    nimble_meshconn_nt_state_t state;
    int handle; // TODO: really needed here?
    ble_addr_t addr;
    int8_t rssi;
    uint8_t of_type;
    uint32_t of;
    uint32_t last_update;
} nimble_meshconn_nt_peer_t;

typedef void (*nimble_meshconn_nt_cb_t)(uint8_t event, nimble_meshconn_nt_peer_t *peer);

typedef struct {
    list_node_t node;
    nimble_meshconn_nt_cb_t cb;
} nimble_meshconn_nt_sub_t;




int nimble_meshconn_nt_init(void);

int nimble_meshconn_nt_activate(void);
int nimble_meshconn_nt_deactivate(void);

int nimble_meshconn_nt_discovered(const ble_addr_t *addr, int rssi,
                                  uint8_t of_type, uint32_t of);
// int nimble_meshconn_nt_state_update(const ble_addr_t *addr, int handle,
//                                     nimble_meshconn_nt_state_t state);

nimble_meshconn_nt_peer_t *nimble_meshconn_nt_best(nimble_meshconn_nt_state_t state);
// nimble_meshconn_nt_peer_t *nimble_meshconn_nt_by_handle(int handle);
uint32_t nimble_meshconn_nt_peer_of(int handle);

// int nimble_meshconn_nt_set_of_type(uint8_t of_type);
int nimble_meshconn_nt_purge(void);


#ifdef __cplusplus
}
#endif

#endif /* NIMBLE_MESHCONN_NT_H */
/** @} */
