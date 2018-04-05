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

/* TDODO: move SIG services into the Gorm stack at some point? */
#ifdef MODULE_GGS_IPS
static gorm_gatt_entry_t ips_entry;
extern gorm_gatt_service_t ggs_ips_service;
#endif

#ifdef MODULE_GGS_RIOT_LED
static gorm_gatt_entry_t led_entry;
extern gorm_gatt_service_t ggs_riot_led_service;
#endif

void ggs_init(void)
{
#ifdef MODULE_GGS_IPS
    gorm_gatt_tab_reg_service(&ips_entry, &ggs_ips_service);
#endif

#ifdef MODULE_GGS_RIOT_LED
    gorm_gatt_tab_reg_service(&led_entry, &ggs_riot_led_service);
#endif
/* TODO: add SAUL etc ... */
}
