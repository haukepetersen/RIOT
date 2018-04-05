/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm_arch
 * @{
 *
 * @file
 * @brief       Mapping of the platform dependent (pseudo-)random number
 *              generator
 *
 * @todo        Think about inlining...
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_ARCH_RAND_H
#define GORM_ARCH_RAND_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Generate a random number that lies within the [min,max)-interval
 *
 * @param[in] min   lower bound of output values (including this value)
 * @param[in] max   upper bound of output values (excluding this value)
 *
 * @return  (pseudo-)random value from the [min,max)-interval
 */
uint32_t gorm_arch_rand(uint32_t min, uint32_t max);

#ifdef __cplusplus
}
#endif

#endif /* RANDOM_H */
/** @} */
