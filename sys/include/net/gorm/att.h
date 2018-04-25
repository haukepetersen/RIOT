/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_att ATT for Gorm
 * @ingroup     net_gorm
 * @brief       Gorm's ATT (Attribute protocol) implementation
 * @{
 *
 * @file
 * @brief       Gorm's ATT (Attribute Protocol) definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_GORM_ATT_H
#define NET_GORM_ATT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    ATT Permission flags
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

#endif /* NET_GORM_ATT_H */
/** @} */
