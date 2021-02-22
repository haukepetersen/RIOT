/*
 * Copyright (C) 2019 Freie Universität Berlin
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
 * @brief       L2CAP connection oriented channels (COC) for Gorm
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_COC_H
#define GORM_COC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    COC connect response codes
 * @{
 */
#define GORM_COC_RESP_SUCCESS           0x0000
#define GORM_COC_RESP_REFUSED_SPSM      0x0002
#define GORM_COC_RESP_REFUSED_RESSOURCE 0x0004
#define GORM_COC_RESP_REFUSED_AUTHEN    0x0005
#define GORM_COC_RESP_REFUSED_AUTHOR    0x0006
#define GORM_COC_RESP_REFUSED_KEYSIZE   0x0007
#define GORM_COC_RESP_REFUSED_ENC       0x0008
#define GORM_COC_RESP_REFUSED_SRC_CID   0x0009
#define GORM_COC_RESP_REFUSED_CID_INUSE 0x000a
#define GORM_COC_RESP_REFUSED_INV_PARM  0x000b
/** @} */

#define GORM_L2CAP_COC_CONN_REQ_PKT_LEN 10U
#define GORM_L2CAP_COC_CONN_RSP_PKT_LEN 10U



typedef void (*gorm_coc_cb_t)(gorm_coc_t *coc);


/**
 * @internal
 * COC: receiving data
 * - ll gets L2cap packet (gorm_buf_t), passed to l2cap layer
 * - l2cap parses packet (gorm_l2cap_on_data)
 *   - get CID field from L2CAP header
 *   - search cocs list for that CID entry
 *   - call gorm_coc_on_data(coc, gorm_buf) for that coc
 */
void gorm_coc_on_data(gorm_ctx_t *con, gorm_buf_t *buf,
                      uint16_t cid, uint8_t llid,
                      uint8_t *data, size_t data_len);

int gorm_coc_on_con_req(gorm_ctx_t *con, gorm_buf_t *buf,
                        uint8_t *data, size_t size);

void gorm_coc_read_to_flat(gorm_coc_t *coc, void *buf);


int gorm_coc_send(gorm_coc_t *coc, iolist_t *data);

int gorm_coc_send_flat(gorm_coc_t *coc, void *data, size_t len);




/**
 * @brief   Prepare Gorm to accept an incoming L2CAP COC connection request for
 *          the given CID
 *
 * @params[out] con         connection
 * @params[out] coc         allocated memory for the COC context
 * @params[in] psm          protocol service multiplexer value (e.g. 0x0023 for IPSP)
 * @params[in] on_data_cb   on data callback
 * @params[in] arg          user supplied argument passed to @p on_data_cb
 */
int gorm_coc_accept(gorm_ctx_t *con, gorm_coc_t *coc,
                    uint16_t psm, uint16_t mps,
                    gorm_coc_cb_t on_data_cb, void *arg);

int gorm_coc_connect(gorm_ctx_t *con, gorm_coc_t *coc,
                     uint16_t psm, uint16_t mps,
                     gorm_coc_cb_t on_data_cb, void *arg);

// TODO: move to convenience module or the like
/* conveninece function, use instead of gap_connect + coc_connect */
// int gorm_coc_connect_full(gorm_ctx_t *con, gorm_coc_t *coc,
                          // const uint8_t *addr);
/* timings are set globally -> use gorm_gap_x() */


#ifdef __cplusplus
}
#endif

#endif /* GORM_COC_H */
