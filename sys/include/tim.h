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

#ifndef TIM_H
#define TIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "msg.h"
#include "thread.h"
#include "thread_flags.h"
#include "timex.h"

// #include "tim/types.h"
// #include "tim/cfg.h"

/**
 * @name    (Default) peripheral configuration
 * @{
 */
#ifndef TIM_TIMER_DEV
#define TIM_TIMER_DEV       (TIMER_DEV(0))
#endif
#ifndef TIM_TIMER_FREQ
#define TIM_TIMER_FREQ      (1000000)
#endif
#ifndef TIM_TIMER_MASK
#define TIM_TIMER_MASK      (0xffffffff)
#endif
/** @} */

#define TIM_BACKOFF         (1U)

typedef void (*tim_func_t)(void *arg);

typedef enum {
    US_BURST,
    MS_CONT
} tim_rail_t;

typedef struct {
    uint32_t (*now)(void);
    void (*set)(uint32_t to);
    void (*clr)(void);
    uint32_t ticks_per_second;
    uint32_t mask;
    uint8_t num;
} tim_llt_t;

typedef struct tim_base tim_base_t;

struct tim_base {
    tim_base_t *next;
    uint32_t t0;
    uint32_t t1;
    tim_func_t action;
    void *arg;
};

typedef struct {
    tim_base_t base;
    kernel_pid_t pid;
    msg_t msg;
} tim_msg_t;

#if 0
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


typedef struct {
    tim_base_t *queue;
    uint32_t last;
    tim_llt_t *llt;
} tim_timer_t;

extern tim_timer_t tim_burst_us;
extern tim_timer_t tim_cont_ms;


void tim_init(void);

void tim_sleep(tim_rail_t rail, uint32_t to);

void tim_set_cb(tim_rail_t rail, uint32_t to, tim_base_t *t,
                tim_func_t cb, void *arg);

void tim_set_msg(tim_timer_t timer, uint32_t to, tim_msg_t *t,
                 kernel_pid_t pid, msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* TIM_H */
/** @} */
