/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_tim Tim, the RIOT system timer
 * @ingroup     sys
 * @brief       High-level timer interface for RIOT
 * @{
 *
 * @file
 * @brief       High-level timer interface for RIOT
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#include <stdint.h>

#include "msg.h"
#include "thread.h"
#include "thread_flags.h"


typedef void (*tim_func_t)(void *arg);

typedef struct tim_base tim_base_t;

typedef enum {
    US_BURST,
    MS_CONT
} tim_rail_t;

struct tim_base {
    tim_base_t *next;
    uint32_t t0;
    uint32_t t1;
    tim_func_t cb;
};

typedef struct {
    tim_base_t base;
    void *arg;
} tim_cb_t;

#if 0
typedef struct {
    tim_base_t base;
    kernel_pid_t pid;
    msg_t msg;
} tim_msg_t;

typedef strcut {
    time_base_t;
} tim_wakeukp_t;

#ifdef MODULE_CORE_THREAD_FLAGS
typedef struct {
    tim_base_t base;
    thread_t *thread;
    thread_flags_t mask;
} tim_flags_t;
#endif

#endif

void tim_init(void);

void tim_sleep(tim_rail_t rail, uint32_t to);

void tim_set_cb(tim_rail_t rail, uint32_t to, tim_cb_t *t, tim_cb_t cb, void *arg);
