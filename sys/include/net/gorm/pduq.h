/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm
 * @{
 *
 * @file
 * @brief       Gorm's PDU (packet) pool
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_PDUQ_H
#define GORM_PDUQ_H

#include <stdint.h>

#include "net/gorm/buf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GORM_PDUQ_INIT      { NULL, NULL }

typedef struct gorm_pduq {
    gorm_buf_t *head;
    gorm_buf_t *tail;
} gorm_pduq_t;

void gorm_pduq_enq(gorm_pduq_t *queue, gorm_buf_t *pkt);

gorm_buf_t *gorm_pduq_deq(gorm_pduq_t *queue);

static inline gorm_buf_t *gorm_pduq_peek(gorm_pduq_t *queue)
{
    return queue->head;
}

#ifdef __cplusplus
}
#endif

#endif /* GORM_PDUQ_H */
/** @} */
