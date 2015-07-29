/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup  sys_wtimer System Timer
 * @ingroup   sys
 * @brief     Provides a high level abstraction timer module to register
 *            timers, get current system time, and let a thread sleep for
 *            a certain amount of time.
 *
 * The implementation takes one low-level timer that is supposed to run at 1MHz
 * speed and multiplexes it. It can deal with low-level timers with less than
 * 32bit width.
 *
 * Insertion and removal of timers has O(n) complexity with (n) being the
 * number of active timers.  The reason for this is that multiplexing is
 * realized by next-first singly linked lists.
 *
 * @{
 * @file
 * @author Kaspar Schleiser <kaspar@schleiser.de>
 */
#ifndef WTIMER_H
#define WTIMER_H

#include <stdint.h>
#include "msg.h"
#include "periph/timer.h"
#include "timex.h"

/**
 * @def internal define to allow using variables instead of defines
 */
#ifdef WTIMER_TRACE
#include "wtimer_trace.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief wtimer callback type
 */
typedef void (*timer_callback_t)(void*);

/**
 * @brief wtimer timer structure
 */
typedef struct wtimer {
    struct wtimer *next;        /**< reference to next timer in timer lists */
    uint32_t target;            /**< lower 32bit absolute target time */
    uint32_t long_target;       /**< upper 32bit absolute target time */
    timer_callback_t callback;  /**< callback function to call when timer expires */
    void *arg;                  /**< argument to pass to callback function */
} wtimer_t;

/**
 * @brief wtimer initialization function
 *
 * This sets up wtimer. Has to be called once at system boot.
 * If auto_init is enabled, it will call this for you.
 */
void wtimer_init(void);

/**
 * @brief Stop execution of a thread for some time
 *
 * When called from an ISR, this function will spin and thus block the MCU in
 * interrupt context for the specified amount in *seconds*, so don't *ever* use
 * it there,
 *
 * @param in seconds the amount of seconds the thread should sleep
 */
static void wtimer_sleep(uint32_t seconds);

/**
 * @brief Stop execution of a thread for some time
 *
 * When called from an ISR, this function will spin and thus block the MCU for
 * the specified amount in microseconds, so only use it there for *very* short
 * periods, e.g., less than WTIMER_BACKOFF.
 *
 * @param in microseconds the amount of microseconds the thread should sleep
 */
static void wtimer_usleep(uint32_t microseconds);

/**
 * @brief Stop execution of a thread for some time, 64bit version
 *
 * When called from an ISR, this function will spin and thus block the MCU for
 * the specified amount in microseconds, so only use it there for *very* short
 * periods, e.g., less then WTIMER_BACKOFF.
 *
 * @param in microseconds the amount of microseconds the thread should sleep
 */
static inline void wtimer_usleep64(uint64_t microseconds);

/**
 * @brief Stop execution of a thread for some time
 *
 * Don't expect nanosecond accuracy. As of mid 2015, this function just calls
 * wtimer_usleep(nanoseconds/1000).
 *
 * When called from an ISR, this function will spin-block, so only use it there for
 * *very* short periods.
 *
 * @param in seconds the amount of nanoseconds the thread should sleep
 */
static void wtimer_nanosleep(uint32_t nanoseconds);

/**
 * @brief Stop execution of a thread for some time, blocking
 *
 * This function will spin-block, so only use it *very* short periods.
 *
 * @param[in] microseconds the amount of microseconds the thread should spin
 */
static inline void wtimer_spin(uint32_t microseconds);

 /**
 * @brief will cause the calling thread to be suspended until the absolute
 * time (last_wakeup + interval). last_wakeup is set to wtimer_now().
 *
 * This function can be used to create periodic wakeups.
 * @c last_wakeup should be set to wtimer_now() before first call of the function.
 *
 * If the result of (@c last_wakeup) would be in the past, the function sets
 * @c last_wakeup to wtimer_now() and returns immediately.
 *
 * @param[in] last_wakeup base time for the wakeup
 * @param[in] usecs time in microseconds that will be added to last_wakeup
 * @return 0 on success, < 0 on error
 */
int wtimer_usleep_until(uint32_t *last_wakeup, uint32_t usecs);

/**
 * @brief get the current system time as 64bit microsecond value
 *
 * @return current time as 64bit microsecond value
 */
uint64_t wtimer_now64(void);

/**
 * @brief get the current system time into a timex_t
 *
 * @param[out] out pointer to timex_t the time will be written to
 */
void wtimer_now_timex(timex_t *out);

/**
 * @brief Set a timer that sends a message
 *
 * This function sets a timer that will send a message <offset> microseconds from now.
 *
 * The mesage struct specified by msg parameter will not be copied, e.g., it
 * needs to point to valid memory until the message has been delivered.
 *
 *
 * @param[in] timer         timer struct to work with
 * @param[in] offset        microseconds from now
 * @param[in] msg           ptr to msg that will be sent
 * @param[in] target_pid    pid the message will be sent to
 */
void wtimer_set_msg(wtimer_t *timer, uint32_t offset, msg_t *msg, kernel_pid_t target_pid);

/**
 * @brief Set a timer that wakes up a thread
 *
 * This function sets a timer that will wake up a thread when the timer has
 * expired.
 *
 * @param[in] timer         timer struct to work with
 * @param[in] offset        microseconds from now
 * @param[in] pid           pid of the thread that will be woken up
 */
void wtimer_set_wakeup(wtimer_t *timer, uint32_t offset, kernel_pid_t pid);

/**
 * @brief Set a timer to execute a callback at some time in the future
 *
 * Expects timer->callback to be set.
 *
 * The callback specified in the timer struct will be executed <offset> microseconds in
 * the future.
 *
 * @warning BEWARE! Callbacks from wtimer_set() are being executed in interrupt context
 * (unless offset < WTIMER_BACKOFF). DON'T USE THIS FUNCTION unless you know
 * *exactly* what that means.
 *
 * @param[in] timer the timer structure to use
 * @param[in] offset time in microseconds from now specifying that timer's callback's execution time
 */
void wtimer_set(wtimer_t *timer, uint32_t offset);

/**
 * @brief remove a timer
 *
 * @note this function runs in O(n) with n being the number of active timers
 *
 * @param[in] timer ptr to timer structure that will be removed
 *
 * @return 1 on success
 * @return 0 when timer was not active
 */
int wtimer_remove(wtimer_t *timer);

/**
 * @brief wtimer backoff value
 *
 * All timers that are less than WTIMER_BACKOFF microseconds in the future will
 * just spin.
 *
 * This is supposed to be defined per-device in e.g., periph_conf.h.
 */
#ifndef WTIMER_BACKOFF
#define WTIMER_BACKOFF 30
#endif

/**
 * @brief wtimer overhead value
 *
 * This value specifies the time a timer will be late if uncorrected.
 *
 * E.g., with WTIMER_OVERHEAD == 0
 * now=wtimer_now();
 * wtimer_set(&timer, X);
 * overhead=X-now
 *
 * wtimer automatically substracts WTIMER_OVERHEAD from a timer's target time.
 *
 * This is supposed to be defined per-device in e.g., periph_conf.h.
 */
#ifndef WTIMER_OVERHEAD
#define WTIMER_OVERHEAD 20
#endif

#ifndef WTIMER_ISR_BACKOFF
#define WTIMER_ISR_BACKOFF 20
#endif

#ifndef WTIMER_MASK
/**
 * @brief wtimer timer mask
 *
 * This value specifies the mask relative to 0xffffffff that the used timer
 * counts to, e.g., 0xffffffff & ~TIMER_MAXVALUE.
 *
 * For a 16bit timer, the mask would be 0xFFFF0000, for a 24bit timer, the mask
 * would be 0xFF000000. Don't set this for 32bit timers.
 *
 * This is supposed to be defined per-device in e.g., periph_conf.h.
 */
#define WTIMER_MASK 0
#endif

#ifndef WTIMER_USLEEP_UNTIL_OVERHEAD
/**
 * @brief wtimer_usleep_until overhead value
 *
 * This value specifies the time a wtimer_usleep_until will be late
 * if uncorrected.
 *
 * This is supposed to be defined per-device in e.g., periph_conf.h.
 */
#define WTIMER_USLEEP_UNTIL_OVERHEAD 10
#endif

/**
 * @{
 * @brief wtimer internal stuff
 */
#if WTIMER_MASK
extern volatile uint32_t _high_cnt;
#endif

/**
 * @brief returns the (masked) low-level timer counter value.
 */
static inline uint32_t _wtimer_now(void)
{
    return timer_read(WTIMER);
}

/**
 * @brief drop bits of a value that don't fit into the low-level timer.
 */
static inline uint32_t _mask(uint32_t val)
{
    return val & ~WTIMER_MASK;
}

int _wtimer_set_absolute(wtimer_t *timer, uint32_t target);
void _wtimer_set64(wtimer_t *timer, uint32_t offset, uint32_t long_offset);
void _wtimer_sleep(uint32_t offset, uint32_t long_offset);
/** @} */

static inline uint32_t wtimer_now(void)
{
#if WTIMER_MASK
    return _wtimer_now() | _high_cnt;
#else
    return _wtimer_now();
#endif
}

static inline void wtimer_spin_until(uint32_t value) {
    while (_wtimer_now() > value);
    while (_wtimer_now() < value);
}

static inline void wtimer_spin(uint32_t offset) {
    offset = _mask(offset + _wtimer_now());
    wtimer_spin_until(offset);
}

static inline void wtimer_usleep(uint32_t offset)
{
    _wtimer_sleep(offset, 0);
}

static inline void wtimer_usleep64(uint64_t microseconds)
{
    _wtimer_sleep((uint32_t) microseconds, (uint32_t) (microseconds >> 32));
}

static inline void wtimer_sleep(uint32_t seconds)
{
    wtimer_usleep64((uint64_t)seconds*1000000);
}

static inline void wtimer_nanosleep(uint32_t nanoseconds)
{
    _wtimer_sleep(nanoseconds/1000, 0);
}

/** @} */

#if WTIMER_OVERHEAD + WTIMER_USLEEP_UNTIL_OVERHEAD > WTIMER_BACKOFF
#warning (WTIMER_OVERHEAD + WTIMER_USLEEP_UNTIL_OVERHEAD > WTIMER_BACKOFF !!)
#warning This will lead to underruns. Check if tests/wtimer_usleep_until runs through.
#endif

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* WTIMER_H */
