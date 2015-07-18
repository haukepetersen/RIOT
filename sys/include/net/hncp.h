/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_hncp HNCP
 * @ingroup     net
 * @brief       Home Network Control Protocol (HNCP) implementation
 *
 * The Home Network Control Protocol (HNCP) is "an extensible configuration
 * protocol and a set of requirements for home network devices on top of the
 * Distributed Node Consensus Protocol (DNCP). It enables automated
 * configuration of addresses, naming, network borders and the seamless use of a
 * routing protocol." [draft-ietf-homenet-hncp-07]
 *
 * @{
 *
 * @file
 * @brief       API for the Home Network Control Protocol (HNCP)
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_HNCP_H
#define NET_HNCP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thread.h"
#include "stdint.h"
#include "ng_nettype.h"
#include "net/ng_ipv6.h"

#define HNCP_MESSAGE_QUEUE_SIZE         (8U)

#define HNCP_STACKSIZE_DEFAULT          (THREAD_STACKSIZE_DEFAULT + \
                                         (sizeof(msg_t) * \
                                          HNCP_MESSAGE_QUEUE_SIZE))

#define HNCP_PRIO_DEFAULT               (THREAD_PRIORITY_MAIN - 1)

void hncp_init(ng_nettype_t transport, uint16_t port, char *stack, int stacksize, char prio);
int hncp_send(ng_ipv6_addr_t *addr, uint16_t port, uint8_t *data, size_t len);
int hncp_req_node(uint8_t *node_identifier);

#ifdef __cplusplus
}
#endif

#endif /* NET_HNCP_H */
/** @} */
