/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_mia_sock Sock API implementation for MIA
 * @ingroup     mia
 * @brief       Provides an implementation of @ref net_sock
 * @{
 *
 * @file
 * @brief       Sock types for MIA
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef SOCK_TYPES_H
#define SOCK_TYPES_H

#include <stdint.h>

#include "net/mia.h"
#include "net/sock/udp.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sock_udp {
    mia_bind_t bind;
    sock_udp_ep_t remote;
    uint16_t flags;
};

#ifdef __cplusplus
}
#endif

#endif /* SOCK_TYPES_H */
/** @} */
