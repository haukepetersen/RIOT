/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_facemaster RIOT's Facemaster
 * @ingroup     net
 * @brief       TODO
 *
 * # About
 *
 * Something about this module (and its submodules...)
 *
 *
 * # TODOs
 * @todo        bootstrapping -> how to integrate with auto_init (especially
 *              how to automatically initialize 154 face devs)
 *
 * @{
 *
 * @file
 * @brief       Mastering faces for ICN
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_FACEMASTER_H
#define NET_FACEMASTER_H

#include <stdint.h>

#include "net/faceimp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Value to mark a face_id as unused
 */
#define FACEMASTER_UNMAPPED             (0U)

#ifndef FACEMASTER_FACE_LIMIT
#define FACEMASTER_FACE_LIMIT           (32U)
#endif

// TODO: factor out faceimp specific codes?!
enum {
    FACEMASTER_OK       =  0,
    FACEMASTER_NOMEM    = -1,
    FACEMASTER_NOTUP    = -2,
    FACEMASTER_NOFACE   = -3,
    FACEMASTER_FACEERR  = -4,       /* TODO: too generic? */
    FACEMASTER_BUSY     = -5,
    FACEMASTER_NOADDR   = -6,
    FACEMASTER_DEVERR   = -7,
};

typedef int(*facemaster_up_cb_t)(unsigned face, uint16_t flags);
typedef int(*facemaster_down_cb_t)(unsigned face);
typedef int(*facemaster_data_cb_t)(unsigned face, void *data, size_t len);

typedef struct {
    facemaster_up_cb_t up;
    facemaster_down_cb_t down;
    facemaster_data_cb_t data;
} facemaster_hooks_t;

/* to be called from: system initialization (i.e. auto_init) */
int facemaster_init(void);

/* to be called from CCN-lite */
int facemaster_hook(const facemaster_hooks_t *hooks);

/* to be called from: faceimp */
// TODO: how to signal face type (bcast, mcast)? Function param or as part of
//       the faceimp_t struct?!
int facemaster_up(faceimp_t *imp);

/* to be called from: faceimp */
int facemaster_down(faceimp_t *imp);

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @return  0 if face is DOWN
 * @return  != 0 if face is UP
 */
static inline int facemaster_is_up(faceimp_t *imp)
{
    return (imp->face_id != FACEMASTER_UNMAPPED);
}

/* to be called from CCN-lite */
int facemaster_face_kill(unsigned face);

/* to be called from: faceimp */
int facemaster_recv(faceimp_t *imp, void *data, size_t len);

/* to be called from: CCN-lite */
int facemaster_send(unsigned face, void *data, size_t len);


#ifdef __cplusplus
}
#endif

#endif /* NET_FACEMASTER_H */
/** @} */
