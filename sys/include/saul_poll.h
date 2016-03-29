/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_saul_poll Poll SAUL devices
 * @ingroup     sys
 * @brief       Generic module for polling SAUL devices and triggering events
 *
 * @{
 *
 * @file
 * @brief       Interface for the generic SAUL polling module
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef SAUL_POLL_H
#define SAUL_POLL_H

#include "saul.h"
#include "saul_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Message type used for interval complete messages
 */
#define SAUL_POLL_MSG_TYPE      (0x0300)

typedef struct {
    saul_reg_t *dev;
    msg_t msg;
    xtimer_t timer;
    uint32_t interval;
} saul_poll_t;

/**
 * @brief   Start and run the SAUL poll thread
 */
void saul_poll_run(void);

int saul_poll_dev(saul_reg_t *dev, uint32_t interval);

#ifdef __cplusplus
}
#endif

#endif /* SAUL_POLL_H */
/** @} */
