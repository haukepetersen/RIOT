/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     saul_poll
 * @{
 *
 * @file
 * @brief       SAUL relay functions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "msg.h"
#include "thread.h"
#include "saul.h"

#define MSG_Q_SIZE          (8U)
#define STACK_SIZE          (THREAD_STACKSIZE_DEFAULT)
#define STACK_PRIO          (THREAD_PRIORITY_MAIN - 4)

static msg_t mq[MSG_Q_SIZE];

static char stack[STACK_SIZE];

static kernel_pid_t mypid;

static void poll(saul_reg_t *dev)
{
    phydat_t data;
    saul_evt_t event = dev->events;

    /* read the current value */
    saul_reg_read(dev, &data);

    /* try if any event matches */
    while (event) {
        if (event->applies(&data, event->arg)) {
            event->trigger(dev, event, data);
        }
        event = event->next;
    }
}

static void *poll_thread(void *arg)
{
    (void)arg;
    msg_t msg;

    /* initialize the message queue */
    msg_init_queue(mq, MSG_Q_SIZE);
    /* save poll thread PID */
    mypid = thread_getpid();

    while (1) {
        msg_receive(msg);

        if (msg.type == SAUL_POLL_MSG_TYPE) {
            saul_poll_t *polldev = (saul_poll_t *)msg.content.ptr;

            xtimer_set_msg(&polldev->timer, polldev->interval, &polldev->msg, mypid);
            poll(polldev->dev);
        }
    }

}

int saul_poll_run(void)
{
    thread_create(stack, sizeof(stack), STACK_PRIO, 0,
                  poll_thread, NULL, "saul_poll");
}

int saul_poll_dev(saul_poll_t *polldev, saul_reg_t *dev, uint32_t interval)
{
    polldev->dev = dev;
    polldev->interval = interval;
    polldev->msg.type = SAUL_POLL_MSG_TYPE;
    polldev->msg.content.ptr = (void *)polldev;

    xtimer_set_msg(&polldev->timer, polldev->interval, &polldev->msg, mypid);
}

int saul_poll_stop(saul_poll_t *polldev)
{
    /* To clarify: is it possible, that this function is called at exact the
     * moment, in which the poll thread is reacting to a message of this device,
     * so that xtimer_set_msg will be called again, so the timer is not removed ? */
    xtimer_remove(&polldev->timer);
}
