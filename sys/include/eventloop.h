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

#ifndef EVENTLOOP_STACKSIZE
/**
 * @brief   Default stack size for the global event loop thread
 */
#define EVENTLOOP_STACKSIZE             (THREAD_STACKSIZE_DEFAULT)
#endif

#ifndef EVENTLOOP_PRIORITY
/**
 * @brief   Default thread priority of the global event loop thread
 */
#define EVENTLOOP_PRIORITY              (THREAD_PRIORITY_IDLE - 1)
#endif

/**
 * @brief   Expose the global event queue
 */
extern event_queue_t eventloop_queue;

/**
 * @brief   Run the
 */
void eventloop_init(void);

/**
 * @brief   Get a reference to the global event queue
 *
 * @return  pointer to the global event queue
 */
event_queue_t *eventloop_get(void);

/**
 * @brief   Convenience function to post an event to the global event queue
 *
 * @param[in] event     event to execute
 */
static inline void eventloop_post(event_t *event)
{
    event_post(&eventloop_queue, event);
}

/**
 * @brief   Convenience function to remove an event from the global event queue
 *
 * @param[in] event     event to cancel
 */
static inline void eventloop_cancel(event_t *event)
{
    event_cancel(&eventloop_queue, event);
}

#ifdef __cplusplus
}
#endif

#endif /* EVENTLOOP_H */
/** @} */
