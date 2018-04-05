/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm
 * @{
 *
 * @file
 * @brief       Gorm's ATT (Attribute Protocol) implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_ATT_H
#define GORM_ATT_H

#include "net/gorm.h"
#include "net/gorm/uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    ATT error codes
 * @{
 */
/** @} */

/**
 * @name    Attribute opcodes
 * @{
 */
/** @} */

/**
 * @name    Attribute properties
 * @{
 */
/** @} */

/**
 * @name    Permission flags
 *
 * TODO: probably need re-definition...
 * @{
 */
#define GORM_ATT_READABLE                   (0x01)
#define GORM_ATT_WRITABLE                   (0x02)
#define GORM_ATT_ENC_REQ                    (0x04)
#define GORM_ATT_AUTHEN_REQ                 (0x08)
#define GORM_ATT_AUTHOR_REQ                 (0x10)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* GORM_PDUQ_H */
/** @} */
