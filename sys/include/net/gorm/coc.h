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




typedef void (*gorm_coc_cb_t)(gorm_coc_t *coc, void *data, size_t len);


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

void gorm_coc_read_to_flat(gorm_coc_t *coc, void *buf);


int gorm_coc_send(gorm_coc_t *coc, iolist_t *data);

int gorm_coc_send_flat(gorm_coc_t *coc, void *data, size_t len);





int gorm_coc_connect(gorm_ctx_t *con, gorm_coc_t *coc,
                     uint16_t cid, size_t mtu);

/* conveninece function, use instead of gap_connect + coc_connect */
int gorm_coc_connect_full(gorm_ctx_t *con, gorm_coc_t *coc,
                          const uint8_t *addr);
/* timings are set globally -> use gorm_gap_x() */

#ifdef __cplusplus
}
#endif

#endif /* GORM_COC_H */
