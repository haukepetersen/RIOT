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
 * @brief       Gorm's channel selection algorithms
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_LL_CHAN_H
#define NET_GORM_LL_CHAN_H

#include <stdint.h>

#include "net/gorm/ll.h"

/**
 * @brief   Count the allowed channels in the given channel map
 *
 * @param[in] map       map masking the allowed channels
 *
 * @return  number of allowed channels
 */
uint8_t gorm_ll_chan_count(const uint8_t *map);

/**
 * @brief   Compute the next used channel using channel selection algorithm #1
 *
 * @param[in,out] con   connection context
 */
void gorm_ll_chan_algo1(gorm_ll_ctx_t *con);

/**
 * @brief   Compute the next used channel using channel selection algorithm #2
 *
 * @param[in,out] con   connection context
 */
/* TODO: implement */
//void gorm_ll_chan_algo2(gorm_ll_connection_t *con);

#endif /* NET_GORM_CHAN_H */
/** @} */
