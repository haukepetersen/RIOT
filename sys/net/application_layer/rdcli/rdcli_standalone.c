/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_rdcli_simple
 * @{
 *
 * @file
 * @brief       Standalone extension for the simple RD registration client
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdint.h>

#include "log.h"
#include "thread.h"
#include "xtimer.h"
#include "net/rdcli.h"
#include "net/rdcli_config.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

/* stack configuration */
#define STACKSIZE           (THREAD_STACKSIZE_DEFAULT)
#define PRIO                (THREAD_PRIORITY_MAIN - 1)
#define TNAME               "rdcli"

#define UPDATE_TIMEOUT      (0xe537)

#define TIMEOUT_US          ((uint64_t)(RDCLI_UPDATE_INTERVAL * US_PER_SEC))

static char _stack[STACKSIZE];

static xtimer_t _timer;
static kernel_pid_t _runner_pid;
static msg_t _msg;

static void _set_timer(void)
{
    xtimer_set_msg64(&_timer, TIMEOUT_US, &_msg, _runner_pid);
}

static void *_reg_runner(void *arg)
{
    (void)arg;
    msg_t in;

    /* prepare context and message */
    _runner_pid = thread_getpid();
    _msg.type = UPDATE_TIMEOUT;

    while (1) {
        msg_receive(&in);
        if (in.type == UPDATE_TIMEOUT) {
            if (rdcli_update() == RDCLI_OK) {
                _set_timer();
            }
        }
    }

    return NULL;    /* should never be reached */
}

void rdcli_standalone_run(void)
{
    thread_create(_stack, sizeof(_stack), PRIO, 0, _reg_runner, NULL, TNAME);
}

void rdcli_standalone_signal(bool connected)
{
    /* clear timer in any case */
    xtimer_remove(&_timer);
    /* reset the update timer in case a connection was established or updated */
    if (connected) {
        _set_timer();
    }

}
