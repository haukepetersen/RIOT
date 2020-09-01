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

#ifndef HILO_H
#define HILO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HILO_OFFSET         (3)

typedef void(*hilo_cb_t)(void);

typedef struct {
    int hi;
    int lo;
    int maxhi;
    int minlo;
    uint8_t state;
    hilo_cb_t on_pulse;
} hilo_t;


void hilo_init(hilo_t *hilo, hilo_cb_t on_pulse);

void hilo_update(hilo_t *th, int val);

void hilo_sample(hilo_t *th, int val);


#ifdef __cplusplus
}
#endif

#endif /* HILO_H */
/** @} */
