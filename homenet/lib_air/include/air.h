/*
 * Copyright (C) 2020 <devel@haukepetersen.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    foo
 * @ingroup     bar
 * @brief       TODO
 *
 * @{
 * @file
 * @brief       TODO
 *
 * @author      Hauke Petersen <devel@haukepetersen.de>
 */

#ifndef AIR_H
#define AIR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void *air_init(uint8_t type, size_t airdata_len);

void air_start(void);

#ifdef __cplusplus
}
#endif

#endif /* AIR_H */
/** @} */
