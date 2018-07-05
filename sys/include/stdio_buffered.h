/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_stdio_buffered Buffered STDIO
 * @ingroup     sys
 * @brief       Memory buffered STDIO backend
 *
 * @{
 * @file
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef STDIO_BUFFERED_H
#define STDIO_BUFFERED_H

#include "stdio_base.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STDIO_BUFFERED_OUT_BUFSIZE
/**
 * @brief   Default output buffer size
 */
#define STDIO_BUFFERED_OUT_BUFSIZE      (4096U)
#endif

#ifndef STDIO_BUFFERED_IN_BUFSIZE
/**
 * @brief   Default input buffer size
 */
#define STDIO_BUFFERED_IN_BUFSIZE       (256U)
#endif

typedef void(*stdio_buffered_out_cb)(const void *data, size_t len);

int stdio_buffered_reg(stdio_buffered_out_cb on_data_cb);
int stdio_buffered_reg_clear(void);


#ifdef __cplusplus
}
#endif

#endif /* STDIO_BUFFERED_H */
/** @} */
