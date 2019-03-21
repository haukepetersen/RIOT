/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_facemaster
 *
 * # TODO
 * - integration of face groups
 *
 *
 * @{
 *
 * @file
 * @brief       Implementation of the Facemaster
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <limits.h>
#include <string.h>

#include "assert.h"
#include "net/facemaster.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

#define SLOT_MASK       (0xff)
#define ID_SHIFT        (8U)

static faceimp_t *_map[FACEMASTER_FACE_LIMIT];

static uint8_t _face_id_prefix = 0;

static const facemaster_hooks_t *_hooks;

static unsigned _gen_face_id(unsigned slot)
{
    // TODO: reserve some bits for BCAST/MCAST face IDs...
    ++_face_id_prefix;
    /* make sure we always have something different then `0` */
    if (_face_id_prefix == 0) {
        ++_face_id_prefix;
    }
    return ((_face_id_prefix << ID_SHIFT) | slot);
}

static unsigned _slot(unsigned face_id)
{
    return (face_id & SLOT_MASK);
}

static unsigned _find_free_slot(void)
{
    for (unsigned i = 0; i < FACEMASTER_FACE_LIMIT; i++) {
        if (_map[i] == NULL) {
            return i;
        }
    }
    return FACEMASTER_NOMEM;
}

int facemaster_init(void)
{
    memset(_map, 0, sizeof(_map));

    // TODO: initialize the connection with CCN-lite?!

    DEBUG("[facemaster] init: all done\n");
    return FACEMASTER_OK;
}

int facemaster_hook(const facemaster_hooks_t *hooks)
{
    _hooks = hooks;
    return FACEMASTER_OK;
}

int facemaster_up(faceimp_t *imp)
{
    assert(imp);

    DEBUG("[facemaster] new face UP: %p\n", (void *)imp);

    /* make sure the given Imp is not already up */
    if (facemaster_is_up(imp)) {
        return FACEMASTER_OK;
    }

    int slot = _find_free_slot();
    if (slot < 0) {
        return FACEMASTER_NOMEM;
    }

    imp->face_id = _gen_face_id(slot);
    _map[slot] = imp;


    if (_hooks) {
        // TODO: flags?!
        _hooks->up(imp->face_id, 0);
    }

    return FACEMASTER_OK;
}

int facemaster_down(faceimp_t *imp)
{
    assert(imp);

    if (!facemaster_is_up(imp)) {
        return FACEMASTER_NOTUP;
    }

    if (_hooks) {
        _hooks->down(imp->face_id);
    }

    _map[_slot(imp->face_id)] = NULL;
    imp->face_id = FACEMASTER_UNMAPPED;

    return FACEMASTER_OK;
}

int facemaster_face_kill(unsigned face)
{
    if (_slot(face) >= FACEMASTER_FACE_LIMIT) {
        return FACEMASTER_NOFACE;
    }

    faceimp_t *imp = _map[_slot(face)];
    if ((imp == NULL) || (imp->face_id != face)) {
        return FACEMASTER_NOFACE;
    }
    faceimp_terminate(imp);

    return FACEMASTER_OK;
}

int facemaster_recv(faceimp_t *imp, void *data, size_t len)
{
    assert(imp);
    assert(data && (len > 0));  /* as of now, no support for empty packets */

    // TODO: call CCN-lite's receive data function (whatever that may be) */
    /* ccnl_process_data(imp->face_id, data, len); */
    if (_hooks) {
        _hooks->data(imp->face_id, data, len);
    }

    return FACEMASTER_OK;
}

/* to be called by CCN-lite */
int facemaster_send(unsigned face, void *data, size_t len)
{
    assert(data && (len > 0));  /* as of now, no support for empty packets */

    if (_slot(face) >= FACEMASTER_FACE_LIMIT) {
        return FACEMASTER_NOFACE;
    }

    faceimp_t *imp = _map[_slot(face)];
    if ((imp == NULL) || (imp->face_id != face)) {
        return FACEMASTER_NOFACE;
    }

    return faceimp_send(imp, data, len);
}
