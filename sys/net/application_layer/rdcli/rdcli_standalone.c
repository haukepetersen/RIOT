/*
 * Copyright (C) 2017 Freie Universität Berlin
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

/* delay values (in seconds) */
#define DELAY_STARTUP       (3U)

/* stack configuration */
#define STACKSIZE           (THREAD_STACKSIZE_DEFAULT)
#define PRIO                (THREAD_PRIORITY_MAIN - 1)
#define TNAME               "rdcli"

enum {
    UNREGSITERED,
    ASSOCIATED
};

static char stack[STACKSIZE];

static void *reg_runner(void *arg)
{
    (void)arg;

    xtimer_sleep(DELAY_STARTUP);

    int state = UNREGSITERED;
    uint32_t delay = 1;

    while (1) {
        switch (state) {
            case UNREGSITERED:
                /* try to register to RD */
                if (rdcli_register(NULL) == RDCLI_OK) {
                    DEBUG("standalone: successfully registered\n");
                    state = ASSOCIATED;
                    delay = RDCLI_UPDATE_INTERVAL;
                }
                else {
                    /* if not successful, do an exponential back-off. We might
                     * be overshooting the UPDATE_INTERVAL value a little bit,
                     * but that should be fine */
                    if (delay < (RDCLI_UPDATE_INTERVAL)) {
                        delay *= 2;
                    }
                    DEBUG("standalone: registration failed, new delay is %i\n", (int)delay);
                }
                break;
            case ASSOCIATED:
                if (rdcli_update() != RDCLI_OK) {
                    DEBUG("standalone: update failed\n");
                    state = UNREGSITERED;
                    delay = 1;
                }
                else {
                    DEBUG("standalone: successfully updated\n");
                }
                break;
        }

        xtimer_sleep(delay);
    }

    return NULL;    /* should never be reached */
}

void rdcli_run(void)
{
    thread_create(stack, sizeof(stack), PRIO, 0, reg_runner, NULL, TNAME);
}
