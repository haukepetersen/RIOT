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
 * @brief       Gorm's buffer management
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_BUF_H
#define NET_GORM_BUF_H

#include "assert.h"
#include "net/netdev/ble.h"

/**
 * @brief   Static initializer for buffer queues (gorm_bufq_t)
 */
#define GORM_BUFQ_INIT      { NULL, NULL }

/**
 * @brief   Generic packet buffer structure used throughout Gorm
 */
typedef struct gorm_pdu {
    struct gorm_pdu *next;
    netdev_ble_pkt_t pkt;
} gorm_buf_t;

/**
 * @brief   Queue containing Gorm buffers
 */
typedef struct {
    gorm_buf_t *head;
    gorm_buf_t *tail;
} gorm_bufq_t;

/**
 * @brief   Global queue used as pool for unused packet buffers
 */
extern gorm_bufq_t gorm_buf_pool;

/**
 * @brief   Add memory to the global buffer pool
 *
 * @pre     (mem != NULL)
 *
 * @param[in] mem       pointer to a block of memory
 * @param[in] len       length of the available memory, should be a multiple of
 *                      sizeof(gorm_buf_t)
 */
void gorm_buf_addmem(void *mem, size_t len);

/**
 * @brief   Count the number of elements in the given queue
 *
 * @pre     (queue != NULL)
 *
 * @param[in] queue     queue to count
 *
 * @return  number of elements in the given queue
 */
size_t gorm_buf_count(const gorm_bufq_t *queue);

/**
 * @brief   Insert the given buffer into the given FIFO queue
 *
 * @pre     (queue != NULL) && (pkt != NULL)
 *
 * @param[in] queue     queue the buffer is added to
 * @param[in] pkt       packet to add to the queue
 */
void gorm_buf_enq(gorm_bufq_t *queue, gorm_buf_t *pkt);

/**
 * @brief   Get the first element of the given FIFO queue
 *
 * @pre     (queue != NULL)
 *
 * @param[in] queue     queue to read from
 *
 * @return  first element of the queue
 * @return  NULL if queue is empty
 */
gorm_buf_t *gorm_buf_deq(gorm_bufq_t *queue);

/**
 * @brief   Get the first element of the given queue without removing it from
 *          the queue
 *
 * @pre     (queue != NULL)
 *
 * @param[in] queue     queue to read from
 *
 * @return  first element of the queue
 * @return  NULL if queue is empty
 */
static inline gorm_buf_t *gorm_buf_peek(gorm_bufq_t *queue)
{
    assert(queue);
    return queue->head;
}

/**
 * @brief   Get one buffer from the global buffer pool
 *
 * @return  buffer retrieved from global pool
 * @return  NULL if no buffer is available
 */
static inline gorm_buf_t *gorm_buf_get(void)
{
    return gorm_buf_deq(&gorm_buf_pool);
}

/**
 * @brief   Return the given buffer to the global buffer pool
 *
 * @pre     (buf != NULL)
 *
 * @param[in] buf   buffer to return to global pool
 */
static inline void gorm_buf_return(gorm_buf_t *buf)
{
    assert(buf);
    gorm_buf_enq(&gorm_buf_pool, buf);
}

#endif /* NET_GORM_BUF_H */
/** @} */