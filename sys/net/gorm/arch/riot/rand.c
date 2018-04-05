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
 * @brief       RIOT specific implementation of Gorm's pseudo random number
 *              generator interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "random.h"

uint32_t gorm_arch_rand(uint32_t min, uint32_t max)
{
    return random_uint32_range(min, max);
}
