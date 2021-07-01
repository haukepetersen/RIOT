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

#define ENABLE_DEBUG    0
#include "debug.h"

#if IS_USED(MODULE_MYPRINT)
#include "myprint.h"
#else
#define myprintf(...)
#endif

static nimble_meshconn_of_t *_ofs = NULL;
static nimble_meshconn_of_t *_active = NULL;

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
    myprintf("[of] activate %u\n", (unsigned)type);

    // TODO: handle
    //  1. new type is not equal the currently active type
    //  2. same type already active -> DONE
    if (_active != NULL) {
        if (_active->type == type) {
            myprintf("[of] already active %u\n", (unsigned)type);
            return -EALREADY;
        }
        else {
            // TODO: handle case where active type changes
            //  -> what happens to downstream links?
            //  -> what happens to other upstream links?
            //  I guess we simply do not allow this!
            myprintf("[of] bad type %u\n", (unsigned)type);
            return -ENOTSUP;
        }
    }

    nimble_meshconn_of_t *of = _get_of(type);
    if (of == NULL) {
        DEBUG("[of] activate %u fail -> entry not found\n", (unsigned)type);
        return -ENOENT;
    }
    _active = of;
    _active->activate(role);

    return 0;
}

int nimble_meshconn_of_deactivate(void)
{
    if (_active == NULL) {
        myprintf("[of] deactivate: not active\n");
        return -EALREADY;
    }

    _active->deactivate();
    _active = NULL;

    return 0;
}

uint32_t nimble_meshconn_of_get(void)
{
    if (_active == NULL) {
        return NIMBLE_MESHCONN_OF_UNDEF;
    }
    return _active->get_of();
}

uint8_t nimble_meshconn_of_get_type(void)
{

    if (_active == NULL) {
        return NIMBLE_MESHCONN_OF_NONE;
    }
    return _active->type;
}

bool nimble_meshconn_of_is_supported(uint8_t of_type)
{
    return (_get_of(of_type) != NULL) ? true : false;
}
