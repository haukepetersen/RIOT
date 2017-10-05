/*
 * Copyright (C) 2017 Kaspar Schleiser <kaspar@schleiser.de>
 *               2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys
 * @{
 *
 * @file
 * @brief       Over the air (OTA) update TFTP server
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Francisco Acosta <francisco.acosta@inria.fr>
 *
 * @}
 */

#ifndef OTA_TFTP_SERVER_H
#define OTA_TFTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thread.h"

/* the message queues */
#define TFTP_QUEUE_SIZE        (4)

#define OTA_TFTP_SERVER_STACK  (THREAD_STACKSIZE_MAIN + THREAD_EXTRA_STACKSIZE_PRINTF + 4096)

/**
 * @brief  Init function for the ota_update_tftp server module
 *
 * @return The kernel PID for the running module
 *
 */
kernel_pid_t ota_tftp_server_start(void);

void ota_tftp_server_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* OTA_TFTP_SERVER_H */
