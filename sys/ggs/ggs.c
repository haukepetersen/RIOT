/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_ggs
 * @{
 *
 * @file
 * @brief       Bootstrapping of included GATT services for GORM
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "net/gorm/gatt.h"
#include "net/gorm/gatt/tab.h"

#include "net/ggs.h"
#include "net/ggs/riot.h"
#include "net/ggs/sig.h"

void ggs_init(void)
{
#ifdef MODULE_GGS_IPS
    gorm_gatt_tab_reg_service(&ggs_ips_service);
#endif

#ifdef MODULE_GGS_RIOT_LED
    gorm_gatt_tab_reg_service(&ggs_riot_led_service);
#endif
/* TODO: add SAUL etc ... */
}
