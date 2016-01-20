/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   drivers_saul
 * @{
 *
 * @file
 * @brief     Default SAUL GPIO configuration
 *
 * @author    Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef SAUL_GPIO_PARAMS_H
#define SAUL_GPIO_PARAMS_H

#include "board.h"
#include "saul/periph.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SAUL_GPIO_BOARD
static const saul_gpio_params_t saul_gpio_params[] = {
    SAUL_GPIO_BOARD
};
#endif

#endif /* SAUL_GPIO_PARAMS_H */
/** @} */
