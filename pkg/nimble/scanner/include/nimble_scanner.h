/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    pkg_nimble_scanner
 * @ingroup     pkg_nimble
 * @brief       TODO
 * @{
 *
 * @file
 * @brief       Scanner abstraction for NimBLE
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef PKG_NIMBLE_SCAN_H
#define PKG_NIMBLE_SCAN_H

#include <stdint.h>

#include "host/ble_hs.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    NIMBLE_SCANNER_OK       =  0,
    NIMBLE_SCANNER_SCANNING =  1,
    NIMBLE_SCANNER_STOPPED  =  2,
    NIMBLE_SCANNER_ERR      = -1,
};

typedef void(*nimble_scanner_cb)(const ble_addr_t *addr, int8_t rssi,
                                 const uint8_t *ad, size_t ad_len);

extern struct ble_npl_eventq *ble_hs_evq_get(void);

int nimble_scanner_start(void);

void nimble_scanner_stop(void);

int nimble_scanner_status(void);

int nimble_scanner_init(const struct ble_gap_disc_params *params,
                        nimble_scanner_cb disc_cb, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* PKG_NIMBLE_SCAN_H */
/** } */
