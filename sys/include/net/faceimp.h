/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_facemaster_faceimp RIOT's Face abstraction
 * @ingroup     net_facemaster
 * @brief       TODO
 * @{
 *
 * @file
 * @brief       Generic face implementation interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_FACEIMP_H
#define NET_FACEIMP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Return values used by Faceimps
 */
enum {
    FACEIMP_OK      =  0,
    FACEIMP_NOMEM   = -1,
    FACEIMP_DEVERR  = -2,
    FACEIMP_NOTCONN = -3,
    FACEIMP_NOTSUP  = -4,
};

typedef struct faceimp faceimp_t;

/* called from facemaster */
typedef int(*faceimp_send_t)(faceimp_t *imp, void *data, size_t len);
/* called from facemaster */
typedef int(*faceimp_terminate_t)(faceimp_t *imp);

/* called from faceimp itself, notifies user (or faceman) about state changes */
typedef void(*faceimp_signal_t)(faceimp_t *imp, unsigned event);

typedef struct {
    faceimp_send_t send;
    faceimp_terminate_t terminate;
} faceimp_hooks_t;

struct faceimp {
    unsigned face_id;
    const faceimp_hooks_t *hooks;
};

/* convenience function */
static inline int faceimp_send(faceimp_t *imp, void *data, size_t len)
{
    return imp->hooks->send(imp, data, len);
}

/* convenience function */
static inline void faceimp_terminate(faceimp_t *imp)
{
    imp->hooks->terminate(imp);
}

#ifdef __cplusplus
}
#endif

#endif /* NET_FACEIMP_H */
/** @} */
