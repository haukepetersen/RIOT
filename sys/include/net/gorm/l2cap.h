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
#define GORM_L2CAP_LE_PSM_IPSP      (0x0023)
/** @} */

/**
 * @brief   L2CAP header length in bytes
 */
#define GORM_L2CAP_HDR_LEN          (4U)

typedef void (*gorm_l2cap_coc_cb_t)(gorm_coc_t *coc, void *data, size_t len);

typedef struct gorm_l2cap_coc {
    struct gorm_l2cap_coc *next;
    gorm_ctx_t *con;        /**< connection handle, if NULL than coc is not connected*/
    uint16_t psm;
    uint16_t cid_src;
    uint16_t cid_dst;
    uint16_t rx_len;
    uint16_t pos;
    uint16_t credits;
    uint16_t peer_mps;
    uint16_t peer_mtu;
    gorm_bufq_t rxq;
    uint8_t *rb;            /**< reassembly buffer */
} gorm_l2cap_coc_t;

typedef struct {
    gorm_l2cap_coc_t *cocs;
} gorm_l2cap_ctx_t;


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
 * @internal
 *
 * @param[in] con       connection context the data came in from
 * @param[in] llid      actual link layer packet type (CONT or START)
 * @param[in] data      pointer to the incoming packet
 */
void gorm_l2cap_on_data(gorm_ctx_t *con, uint8_t llid, gorm_buf_t *data);






/**
 * @internal
 * COC: receiving data
 * - ll gets L2cap packet (gorm_buf_t), passed to l2cap layer
 * - l2cap parses packet (gorm_l2cap_on_data)
 *   - get CID field from L2CAP header
 *   - search cocs list for that CID entry
 *   - call gorm_coc_on_data(coc, gorm_buf) for that coc
 */
void gorm_coc_on_data(gorm_coc_t *coc, uint8_t llid?, gorm_buf_t *data);

int gorm_coc_send(gorm_coc_t *coc, void *data, size_t len);

int gorm_coc_connect(gorm_ctx_t *con, gorm_l2cap_coc_t *coc,
                     uint16_t cid, size_t mtu);

/* conveninece function, use instead of gap_connect + coc_connect */
int gorm_coc_connect_full(gorm_ctx_t *con, gorm_l2cap_coc_t *coc,
                          const uint8_t *addr);
/* timings are set globally -> use gorm_gap_x() */


#ifdef __cplusplus
}
#endif

#endif /* GORM_L2CAP_H */
/** @} */
