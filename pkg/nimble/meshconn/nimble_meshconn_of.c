/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble_meshconn_of
 * @{
 *
 * # About
 * Cares for calculating our own current OF only!
 *
 * @file
 * @brief       Meshconn objective function: ICMP echo (ping)
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <errno.h>

#include "list.h"
#include "nimble_meshconn.h"
#include "nimble_meshconn_of.h"

#define ENABLE_DEBUG    1
#include "debug.h"

static nimble_meshconn_of_t *_ofs = NULL;
// static nimble_meshconn_of_t *_active = NULL;

list_node_t nimble_meshconn_of_subs = { NULL };


static nimble_meshconn_of_t *_get_of(uint8_t type)
{
    nimble_meshconn_of_t *tmp = _ofs;
    while (tmp) {
        if (tmp->type == type) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

int nimble_meshconn_of_register(uint8_t type, nimble_meshconn_of_t *of)
{
    /* make sure the given type is not registered yet */
    nimble_meshconn_of_t *tmp = _get_of(type);
    if (tmp != NULL) {
        return -EALREADY;
    }

    /* insert new objective function */
    of->type = type;
    of->next = _ofs;
    _ofs = of;

    DEBUG("[of] register %u ok\n", (unsigned)type);

    return 0;
}

int nimble_meshconn_of_activate(uint8_t type, int role)
{
    DEBUG("[of] activate %u\n", (unsigned)type);

    nimble_meshconn_of_t *of = _get_of(type);
    if (of == NULL) {
        DEBUG("[of] activate %u fail -> entry not found\n", (unsigned)type);
        return -ENOENT;
    }
    of->activate(role);

    return 0;
}

int nimble_meshconn_of_deactivate(uint8_t type)
{
    nimble_meshconn_of_t *of = _get_of(type);
    if (of == NULL) {
        return -ENOENT;
    }

    of->deactivate();

    return 0;
}

void nimble_meshconn_of_notify(uint8_t of_type, uint32_t of)
{
    list_node_t *node = nimble_meshconn_of_subs.next;
    while (node) {
        nimble_meshconn_of_sub_t *sub = (nimble_meshconn_of_sub_t *)node;
        sub->cb(of_type, of);
        node = node->next;
    }
}
