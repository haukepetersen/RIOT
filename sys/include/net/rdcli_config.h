/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_rdcli_config CoRE RD Client Configuration
 * @ingroup     net
 * @brief       Shared CoRE Resource Directory Client Configuration
 * @{
 *
 * @file
 * @brief       CoRE RD Client static configuration default values
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_RDCLI_CONFIG_H
#define NET_RDCLI_CONFIG_H

#include "net/ipv6/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default lifetime in seconds (the default is 1 day)
 */
#ifndef RDCLI_LT
#define RDCLI_LT                (86400UL)
#endif

/**
 * @brief   Default client update interval (default is 3/4 the lifetime)
 */
#ifndef RDCLI_UPDATE_INTERVAL
#define RDCLI_UPDATE_INTERVAL   ((RDCLI_LT / 4) * 3)
#endif

/**
 * @name    Endpoint ID definition
 *
 * Per default, the endpoint ID (ep) is generated by concatenation of a user
 * defined prefix (RDCLI_EP_PREFIX) and a locally unique ID (luid) encoded in
 * hexadecimal formatting with the given length of characters
 * (RDCLI_EP_SUFFIX_LEN).
 *
 * Alternatively, the endpoint ID value can be defined at compile time by
 * assigning a string value to the RDCLI_ED macro.
 *
 * @{
 */
#ifndef RDCLI_EP
/**
 * @brief   Number of generated hexadecimal characters added to the ep
 *
 * @note    Must be an even number
 */
#define RDCLI_EP_SUFFIX_LEN     (16)

/**
 * @brief   Default static prefix used for the generated ep
 */
#define RDCLI_EP_PREFIX         "RIOT-"
#endif
/** @} */

/**
 * @brief   Use ALL_NODES multicast address as default address when looking for
 *          a RD server
 */
#ifndef RDCLI_SERVER_ADDR
#define RDCLI_SERVER_ADDR       "ff02::1"
#endif

/**
 * @brief   Default Port to use when looking for RDs
 */
#ifndef RDCLI_SERVER_PORT
#define RDCLI_SERVER_PORT       COAP_PORT
#endif

#ifdef __cplusplus
}
#endif

#endif /* NET_RDCLI_CONFIG_H */
/** @} */
