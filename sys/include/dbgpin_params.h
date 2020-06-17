/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_dbgpin
 *
 * @{
 * @file
 * @brief       Default parameters for the dbgpin helper module
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef DBGPIN_PARAMS_H
#define DBGPIN_PARAMS_H

#include "periph/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef DBGPIN_PARAMS
#define DBGPIN_PARAMS   GPIO_PIN(0,0)
#endif

static const gpio_t dbgpin_params[] = { DBGPIN_PARAMS };

#ifdef __cplusplus
}
#endif

#endif /* DBGPIN_PARAMS_H */
/** @} **/
