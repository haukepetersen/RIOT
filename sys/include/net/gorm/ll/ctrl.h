/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm
 * @brief
 * @{
 *
 * @file
 * @brief       Gorm's interface for handling link layer control messages
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_LL_CTRL_H
#define GORM_LL_CTRL_H

#include "net/gorm/ll.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Build a response to a FEATURE_REQ packet
 *
 * @param[out] pkt      response is written to this packet
 */
void gorm_ll_ctrl_feature_resp(gorm_buf_t *buf);

void gorm_ll_ctrl_version(gorm_buf_t *buf);

void gorm_ll_ctrl_1b(gorm_buf_t *buf, uint8_t opcode, uint8_t data);


void gorm_ll_ctrl_on_data(gorm_ll_connection_t *con, gorm_buf_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* GORM_LL_CTRL_H */
/** @} */
