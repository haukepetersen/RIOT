/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_eventloop Global shared event loop
 * @ingroup     sys
 * @brief       Runs a thread to provide a shared execution context for events
 *
 * This module provides a standalone thread running an event loop. This thread
 * can be used to offload none time critical actions that need a thread context
 * for execution. This can be for example the regular sending of network
 * requests, low-priority housekeeping functions, or any other (none critical)
 * function for which it does not make sense to allocate a dedicated thread.
 *
 * When posting events to the global event loop, it is important to note that
 * there is no kind of guarantee on when the posted action will be processed. In
 * the default configuration, the global event loop thread has the lowest
 * possible priority (one below the idle thread), leading to posted events only
 * being processed if the CPU has nothing else to do.
 *
 * On top, the following rules should be followed when posting events to the
 * queue:
 * - don't do anything that is blocking the thread for a long time: typical
 *   network requests (e.g. a confirmable CoAP request) are the upper bound
 * - use the eventloop only for occasional tasks: if you plan on heavy
 *   utilization better use a dedicated event loop for your module
 * - make sure to adjust the stacksize depending on your usage scenario
 *
 * @{
 *
 * @file
 * @brief       Global, shared event loop interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "event.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EVENTTHREAD_STACKSIZE
/**
 * @brief   Default stack size for the global event loop thread
 */
#define EVENTTHREAD_STACKSIZE       (THREAD_STACKSIZE_DEFAULT)
#endif

#ifndef EVENTLOOP_PRIO_DRIVERS
/**
 * @brief   Default thread priority of the global event loop thread
 */
#define EVENTLOOP_PRIO_DRIVERS      (THREAD_PRIORITY_IDLE - 1)
#endif

#ifndef EVENTLOOP_PRIO_DRIVERS
/**
 * @brief   Default thread priority of the global event loop thread
 */
#define EVENTLOOP_PRIO_DRIVERS      (THREAD_PRIORITY_MAIN - 4)
#endif

typedef enum {
#ifdef MODULE_EVENTTHREAD_LOWPRIO
    EVENTTHREAD_LOWPRIO,
#endif
#ifdef MODULE_EVENTTHREAD_DRIVERS
    EVENTTHREAD_DRIVERS,
#endif
    EVENTTHREAD_COUNT,
} eventthread_t;

/**
 * @brief   Expose the global event queue
 */
extern event_queue_t eventloop_queue;

/**
 * @brief   Run all configured event threads
 */
void eventthread_init(void);

/**
 * @brief   Convenience function to post an event to a global event thread
 *
 * @param[in] target    post the given event to this event thread's queue
 * @param[in] event     event to execute
 */
void eventthread_post(eventthread_t target, event_t *event);

/**
 * @brief   Convenience function to remove an event from the global event queue
 *
 * @param[in] target    remove the given event from this event thread's queue
 * @param[in] event     event to cancel
 */
void eventloop_cancel(eventthread_t target, event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* EVENTLOOP_H */
/** @} */
