/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_ggs GATT Services for Gorm
 * @ingroup     net
 * @brief       Collection of pre-defined GATT services to be used with Gorm
 * @{
 *
 * @file
 * @brief       Bootstrapping interface for the built-in Gorm GATT services
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_SERVICE_RIOT_H
#define GORM_SERVICE_RIOT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialize all configured built-in services and register them with
 *          the GATT table
 */
void ggs_init(void);

#ifdef __cplusplus
}
#endif

#endif /* GORM_SERVICE_RIOT_H */
/** @} */
