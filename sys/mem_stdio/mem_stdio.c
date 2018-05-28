/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_mem_stdio
 * @{
 *
 * @file
 * @brief       Memory mapped STDIO implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "assert.h"
#include "mem_stdio.h"
#include "ringbuffer.h"

static char _out_mem[MEM_STDIO_OUT_BUFSIZE];
static tsrb_t _out_rb;
static mutex_t _out_lock = MUTEX_INIT;

static char _in_mem[MEM_STDIO_IN_BUFSIZE];
static isrpipe_t _in_pipe;

static mem_stdio_cb_t _cb;


void mem_stdio_init(void)
{
    /* might be overkill, but also prevents some very ugly latent failures in
     * case the size values are not powers of two */
    assert(MEM_STDIO_IN_BUFSIZE >= 2);
    assert(bitarithm_bits_set(MEM_STDIO_IN_BUFSIZE) == 1);
    assert(MEM_STDIO_OUT_BUFSIZE >= 2);
    assert(bitarithm_bits_set(MEM_STDIO_OUT_BUFSIZE) == 1);

    isrpipe_init(&_read_pipe, _in_mem, MEM_STDIO_IN_BUFSIZE);
    tsrb_init(&_out_rb, _out_mem, MEM_STDIO_OUT_BUFSIZE);
}

void mem_stdio_register(mem_stdio_cb_t cb)
{

}

ssize_t mem_stdio_read_output(char *buffer, size_t maxlen)
{
    mutex_lock(&_out_lock);
    int n = tsrb_get(&_out_rb, buffer, maxlen);
    mutex_unlock(&_out_lock);
    return (ssize_t)n;
}

void mem_stdio_write_input(const char *buffer, size_t len)
{

}



void uart_stdio_init(void)
{

}

int uart_stdio_read(char* buffer, int count)
{
    return isrpipe_read(&uart_stdio_isrpipe, buffer, count);
}

int uart_stdio_write(const char* buffer, int len)
{
    return tsrb_add(&_out_lock, buffer, len);
}
