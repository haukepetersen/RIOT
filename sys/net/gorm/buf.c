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
 * @brief       Implementation of Gorm's buffer handling
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "irq.h"

#include "net/gorm/buf.h"

/* allocate the global buffer pool */
gorm_bufq_t gorm_buf_pool = GORM_BUFQ_INIT;

void gorm_buf_addmem(void *mem, size_t len)
{
    assert(mem);

    size_t cnt = (len / sizeof(gorm_buf_t));
    gorm_buf_t *buffers = (gorm_buf_t *)mem;

    unsigned is = irq_disable();
    for (size_t i = 0; i < cnt; i++) {
        gorm_buf_enq(&gorm_buf_pool, &buffers[i]);
    }
    irq_restore(is);
}

size_t gorm_buf_count(const gorm_bufq_t *queue)
{
    assert(queue);

    size_t cnt = 0;
    unsigned is = irq_disable();
    gorm_buf_t *buf = queue->head;
    while (buf != NULL) {
        ++cnt;
        buf = buf->next;
    }
    irq_restore(is);

    return cnt;
}

void gorm_buf_enq(gorm_bufq_t *queue, gorm_buf_t *buf)
{
    assert(queue && buf);

    unsigned is = irq_disable();
    /* if list is non-empty: let last element point to buf */
    if (queue->tail) {
        queue->tail->next = buf;
    }
    /* special case: list was empty, so head also points to buf */
    else {
        queue->head = buf;
    }

    buf->next = NULL;
    queue->tail = buf;
    irq_restore(is);
}

gorm_buf_t *gorm_buf_deq(gorm_bufq_t *queue)
{
    assert(queue);

    unsigned is = irq_disable();
    gorm_buf_t *buf = queue->head;
    /* increment next counter only if list is non-empty */
    if (buf) {
        queue->head = buf->next;
        buf->next = NULL;
    }
    /* if list is now empty, also reset tail */
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    irq_restore(is);

    return buf;
}
