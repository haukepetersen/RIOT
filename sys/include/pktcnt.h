/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    TODO
 * @ingroup     TODO
 * @brief       TODO
 * @{
 *
 * @file
 * @brief       TODO
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef PKTCNT_H
#define PKTCNT_H

#include <stdint.h>

#include "net/gnrc/pkt.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PKTCNT_OK = 0,
    PKTCNT_ERR_INIT = -1,
};

int pktcnt_init(void);

void pktcnt_signal(gnrc_pktsnip_t *pkt);

#ifdef __cplusplus
}
#endif

#endif /* PKTCNT_H */
/** @} */
