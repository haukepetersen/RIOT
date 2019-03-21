/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    pkg_nimble_addr Address helper for NimBLE
 * @ingroup     pkg_nimble
 * @brief       NimBLE specific helper functions for handling addresses
 * @{
 *
 * @file
 * @brief       Interface for NimBLE specific address helper functions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef PKG_NIMBLE_ADDR_H
#define PKG_NIMBLE_ADDR_H

#include "nimble/ble.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NIMBLE_ADDR_STR_MAXLEN      (28U)       /* including `\0` char */

void nimble_addr_print(const ble_addr_t *addr);

void nimble_addr_sprint(char *out, const ble_addr_t *addr);

#ifdef __cplusplus
}
#endif

#endif /* PKG_NIMBLE_ADDR_H */
/** @} */
