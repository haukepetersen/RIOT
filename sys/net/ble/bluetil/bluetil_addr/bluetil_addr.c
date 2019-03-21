/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_bluetil_addr
 * @{
 *
 * @file
 * @brief       Implementation of generic BLE address helper functions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <string.h>
#include <stdio.h>

#include "fmt.h"
#include "assert.h"
#include "net/eui48.h"
#include "net/bluetil/addr.h"

void bluetil_addr_to_str(char *out, const uint8_t *addr)
{
    assert(out);
    assert(addr);

    fmt_byte_hex(out, addr[5]);
    for (int i = 4; i >= 0; i--) {
        out += 2;
        *out++ = ':';
        fmt_byte_hex(out, addr[i]);
    }
    out += 2;
    *out = '\0';
}

void bluetil_addr_print(const uint8_t *addr)
{
    assert(addr);

    char str[BLUETIL_ADDR_STRLEN];
    bluetil_addr_to_str(str, addr);
    printf("%s", str);
}

void bluetil_addr_print_ipv6_iid(const uint8_t *addr)
{
    assert(addr);

    eui64_t iid;
    eui48_to_ipv6_iid(&iid, (const eui48_t *)addr);

    printf("[fe80:");
    for (unsigned i = 0; i < 8; i += 2) {
        printf(":%02x%02x", (int)iid.uint8[i], (int)iid.uint8[i + 1]);
    }
    printf("]");
}
