/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_stdio_buffered
 * @{
 *
 * @file
 * @brief       Memory buffered STDIO backend implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "stdio_buffered.h"

static uint8_t _out_buf[STDIO_BUFFERED_OUT_BUFSIZE];

/* input buffer */
static char _in_buf_mem[STDIO_BUFFERED_IN_BUFSIZE];
isrpipe_t _in_buf = ISRPIPE_INIT(_in_buf_mem);

static stdio_buffered_out_cb _on_out_data_cb = NULL;

int stdio_buffered_reg(stdio_buffered_out_cb on_out_data_cb)
{
    _on_out_data_cb = on_out_data_cb;
}

int stdio_buffered_reg_clear(void)
{
    _on_out_data_cb = NULL;
}



void stdio_init(void)
{
    /* nothing to do here? */
}

ssize_t stdio_read(void* buffer, size_t max_len)
{

}

ssize_t stdio_write(const void* buffer, size_t len)
{

}
