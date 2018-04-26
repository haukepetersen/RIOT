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
 * @brief       Gorm's L2CAP layer interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_L2CAP_H
#define GORM_L2CAP_H

#include "net/gorm.h"
#include "net/gorm/buf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Channel identifiers used by L2CAP for LE-U link layers
 * @{
 */
#define GORM_L2CAP_CID_ATT          (0x0004)
#define GORM_L2CAP_CID_LE_SIGNAL    (0x0005)
#define GORM_L2CAP_CID_SM           (0x0006)
#define GORM_L2CAP_CID_CB_MIN       (0x0040)
#define GORM_L2CAP_CID_CB_MAX       (0x007f)
/** @} */

/**
 * @brief   L2CAP header length in bytes
 */
#define GORM_L2CAP_HDR_LEN          (4U)

/**
 * @brief   Send a reply to an incoming L2CAP packet
 *
 * @param[in] con       connection context used for sending the reply
 * @param[in,out] buf   pointer to the packet buffer containing the reply
 * @param[in] data_len  length of the actual payload contained in @p buf
 */
void gorm_l2cap_reply(gorm_ctx_t *con, gorm_buf_t *buf, uint16_t data_len);

/**
 * @brief   Handle incoming L2CAP data
 *
 * @param[in] con       connection context the data came in from
 * @param[in] llid      actual link layer packet type (CONT or START)
 * @param[in] data      pointer to the incoming packet
 */
void gorm_l2cap_on_data(gorm_ctx_t *con, uint8_t llid, gorm_buf_t *data);

#ifdef __cplusplus
}
#endif

#endif /* GORM_L2CAP_H */
/** @} */
