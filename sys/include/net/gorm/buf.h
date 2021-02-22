/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_buf Gorm's buffer management
 * @ingroup     net_gorm
 * @brief       Memory management for Gorm
 * @{
 *
 * @file
 * @brief       Gorm's buffer management interface
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
 * @brief   Generic packet buffer structure used by Gorm
 */
typedef struct gorm_pdu {
    struct gorm_pdu *next;  /**< pointer to the next buffer in a queue */
    netdev_ble_pkt_t pkt;   /**< packet buffer holding a BLE packet */
} gorm_buf_t;

/**
 * @brief   Queue containing Gorm buffers
 */
typedef struct {
    gorm_buf_t *head;       /**< pointer to the first element in the queue */
    gorm_buf_t *tail;       /**< pointer to the last element in the queue */
} gorm_bufq_t;

/**
 * @brief   Global queue used as pool for unused packet buffers
 */
extern gorm_bufq_t gorm_buf_pool;

/**
 * @brief   Add a number of uninitialized buffers to the global buffer pool
 *
 * @param[in] buffers   array of buffers to add to the pool
 * @param[in] len       number of buffers given in @p buffers
 */
void gorm_buf_add_buffers(gorm_buf_t *buffers, size_t len);

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
 * @param[in] buf       buffer to return to global pool
 */
static inline void gorm_buf_return(gorm_buf_t *buf)
{
    assert(buf);
    gorm_buf_enq(&gorm_buf_pool, buf);
}

/**
 * @brief   Return all buffers in the given queue to the global buffer pool
 *
 * @pre     (queue != NULL)
 *
 * @param[in] queue     empty this queue
 */
void gorm_buf_return_q(gorm_bufq_t *queue);

/**
 * @brief   Move all buffers from the source queue to the destination queue
 */
static inline void gorm_buf_move_q(gorm_bufq_t *dst, gorm_bufq_t *src)
{
    gorm_buf_t *buf = gorm_buf_deq(src);
    while (buf) {
        gorm_buf_enq(dst, buf);
        buf = gorm_buf_deq(src);
    }
}

#endif /* NET_GORM_BUF_H */
/** @} */
