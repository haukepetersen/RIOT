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

#ifndef GORM_PDUPOOL_H
#define GORM_PDUPOOL_H

#include "net/gorm.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief   Initialize the PDU pool
 *
 * @param mem [description]
 * @param len [description]
 */
void gorm_pdupool_addmem(void *mem, size_t len);

/**
 * @brief   Get a number of packets from the pool
 *
 * @param[in] num
 * @return [description]
 */
gorm_buf_t *gorm_pdupool_get(unsigned num);

/**
 * @brief   Free the given packet by putting it back into the pool
 *
 * @param[in,out] pdu   packet to return to pool
 */
void gorm_pdupool_return(gorm_buf_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* GORM_PDUPOOL_H */
/** @} */
