/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Interface for running the test specific UDP echo server
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef APP_UDP_SERVER_H
#define APP_UDP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Run an UDP echo server
 */
void app_udp_server_run(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_UDP_SERVER_H */
/** @} */
