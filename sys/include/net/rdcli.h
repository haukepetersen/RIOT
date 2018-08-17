/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_rdcli CoAP Resource Directory Client
 * @ingroup     net
 * @{
 *
 * @file
 * @brief       Interface for a CoAP RD client
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef RDCLI_H
#define RDCLI_H

#include "net/sock/udp.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    RDCLI_OK        =  0,   /**< everything went as expected */
    RDCLI_TIMEOUT   = -1,   /**< no response from the network */
    RDCLI_ERR       = -2,   /**< internal error or invalid reply */
    RDCLI_NORD      = -3,   /**< not connected to any RD server */
};

/**
 * @brief   Initiate the node registration by sending an empty push
 *
 * - if registration fails (e.g. timeout), we are not associated with any RD
 *   anymore (even if we have been before we called rdcli_register)
 *
 * @return  RDCLI_OK on success
 */
int rdcli_register(sock_udp_ep_t *remote);

/**
 * @brief   Update entries in the given RD server
 */
int rdcli_update(void);

/**
 * @brief   Unregister from a given RD server
 */
int rdcli_remove(void);

/**
 * @brief   Dump the current RD connection status to STDIO (for debugging)
 */
void rdcli_dump_status(void);

/**
 * @brief   Spawn a new thread that registers the node and updates the
 *          registration with all responding RDs
 */
void rdcli_run(void);

#ifdef __cplusplus
}
#endif

#endif /* RDCLI_H */
/** @} */
