/**
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 * @ingroup wtimer
 * @{
 * @file
 * @brief wtimer core functionality
 * @author Kaspar Schleiser <kaspar@schleiser.de>
 * @}
 */
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "periph/timer.h"
#include "periph_conf.h"

#include "wtimer.h"
#include "irq.h"

/* WARNING! enabling this will have side effects and can lead to timer underflows. */
#define ENABLE_DEBUG 0
#include "debug.h"

static volatile uint32_t _long_cnt = 0;
#if WTIMER_MASK
volatile uint32_t _high_cnt = 0;
#endif

static wtimer_t *timer_list_head = NULL;
static wtimer_t *overflow_list_head = NULL;
static wtimer_t *long_list_head = NULL;

static void _add_timer_to_list(wtimer_t **list_head, wtimer_t *timer);
static void _add_timer_to_long_list(wtimer_t **list_head, wtimer_t *timer);
static void _shoot(wtimer_t *timer);
static inline void _lltimer_set(uint32_t target);
static uint32_t _time_left(uint32_t target, uint32_t reference);

static void _timer_callback(void);
static void _periph_timer_callback(int chan);

static inline int _this_high_period(uint32_t target);

void wtimer_init(void)
{
    /* initialize low-level timer */
    timer_init(WTIMER, 1 /* us_per_tick */, _periph_timer_callback);

    /* register initial overflow tick */
    _lltimer_set(0xFFFFFFFF);
}

static void _wtimer_now64(uint32_t *short_term, uint32_t *long_term)
{
    uint32_t before, after, long_value;

    /* loop to cope with possible overflow of wtimer_now() */
    do {
        before = wtimer_now();
        long_value = _long_cnt;
        after = wtimer_now();

    } while(before > after);

    *short_term = after;
    *long_term = long_value;
}

uint64_t wtimer_now64(void)
{
    uint32_t short_term, long_term;
    _wtimer_now64(&short_term, &long_term);

    return ((uint64_t)long_term<<32) + short_term;
}

void _wtimer_set64(wtimer_t *timer, uint32_t offset, uint32_t long_offset)
{
    DEBUG(" _wtimer_set64() offset=%u long_offset=%u\n", (unsigned)offset, (unsigned)long_offset);
    if (! (long_offset)) {
        /* timer fits into the short timer */
        wtimer_set(timer, (uint32_t) offset);
    }
    else {
        _wtimer_now64(&timer->target, &timer->long_target);
        timer->target += offset;
        timer->long_target += long_offset;
        if (timer->target < offset) {
            timer->long_target++;
        }

        int state = disableIRQ();
        _add_timer_to_long_list(&long_list_head, timer);
        restoreIRQ(state);
        DEBUG("wtimer_set64(): added longterm timer (long_target=%u target=%u)\n",
                (unsigned)timer->long_target, (unsigned)timer->target);
    }
}

void wtimer_set(wtimer_t *timer, uint32_t offset)
{
    DEBUG("timer_set(): offset=%u now=%u (%u)\n", (unsigned)offset, (unsigned)wtimer_now(), (unsigned)_wtimer_now());
    if (!timer->callback) {
        DEBUG("timer_set(): timer has no callback.\n");
        return;
    }

    uint32_t now = wtimer_now();
    uint32_t target = now+offset;
    timer->target = target;

    if (offset < WTIMER_BACKOFF) {
        /* spin until timer should be run */
        wtimer_spin_until(target);

        _shoot(timer);
    } else {
        _wtimer_set_absolute(timer, target);
    }
}

static void _periph_timer_callback(int chan)
{
    (void)chan;
    _timer_callback();
}

static void _shoot(wtimer_t *timer)
{
    timer->callback(timer->arg);
}

static inline void _lltimer_set(uint32_t target)
{
    DEBUG("__lltimer_set(): setting %u\n", (unsigned) _mask(target));
    timer_set_absolute(WTIMER, WTIMER_CHAN, _mask(target));
}

int _wtimer_set_absolute(wtimer_t *timer, uint32_t target)
{
    uint32_t now = wtimer_now();
    int res = 0;

    DEBUG("timer_set_absolute(): now=%u target=%u\n", (unsigned)now, (unsigned)target);

    timer->next = NULL;
    if (target >= now && target - WTIMER_BACKOFF < now) {
        /* backoff */
        wtimer_spin_until(target);
        _shoot(timer);
    }

    timer->target = target;

    unsigned state = disableIRQ();
    if ( !_this_high_period(target) ) {
        DEBUG("wtimer_set_absolute(): the timer doesn't fit into the low-level timer's mask.\n");
        timer->long_target = _long_cnt;
        _add_timer_to_long_list(&long_list_head, timer);
    }
    else if (_mask(now) >= target) {
        DEBUG("wtimer_set_absolute(): the timer will expire in the next timer period\n");
        _add_timer_to_list(&overflow_list_head, timer);
    }
    else {
        DEBUG("timer_set_absolute(): timer will expire in this timer period.\n");
        _add_timer_to_list(&timer_list_head, timer);

        if (timer_list_head == timer) {
            DEBUG("timer_set_absolute(): timer is new list head. updating lltimer.\n");
            _lltimer_set(timer->target - WTIMER_OVERHEAD);
        }
    }

    restoreIRQ(state);

    return res;
}

static void _add_timer_to_list(wtimer_t **list_head, wtimer_t *timer)
{
    while (*list_head && (*list_head)->target <= timer->target) {
        list_head = &((*list_head)->next);
    }

    timer->next = *list_head;
    *list_head = timer;
}

static void _add_timer_to_long_list(wtimer_t **list_head, wtimer_t *timer)
{
    while (*list_head
            && (*list_head)->long_target <= timer->long_target
            && (*list_head)->target <= timer->target) {
        list_head = &((*list_head)->next);
    }

    timer->next = *list_head;
    *list_head = timer;
}

static int _remove_timer_from_list(wtimer_t **list_head, wtimer_t *timer)
{
    while (*list_head) {
        if (*list_head == timer) {
            *list_head = timer->next;
            return 1;
        }
        list_head = &((*list_head)->next);
    }

    return 0;
}

int wtimer_remove(wtimer_t *timer)
{
    unsigned state = disableIRQ();
    int res = 0;
    if (timer_list_head == timer) {
        uint32_t next;
        timer_list_head = timer->next;
        if (timer_list_head) {
            /* schedule callback on next timer target time */
            next = timer_list_head->target - WTIMER_OVERHEAD;
        } else {
            next = _mask(0xFFFFFFFF);
        }
        _lltimer_set(next);
    }
    else {
        res = _remove_timer_from_list(&timer_list_head, timer) ||
            _remove_timer_from_list(&overflow_list_head, timer) ||
            _remove_timer_from_list(&long_list_head, timer);
    }

    restoreIRQ(state);

    return res;
}

static uint32_t _time_left(uint32_t target, uint32_t reference)
{
    uint32_t now = _wtimer_now();

    if (now < reference) {
        return 0;
    }

    if (target > now) {
        return target - now;
    }
    else {
        return 0;
    }
}

static inline int _this_high_period(uint32_t target) {
#if WTIMER_MASK
    return (target & WTIMER_MASK) == _high_cnt;
#else
    (void)target;
    return 1;
#endif
}

/**
 * @brief compare two timer's target values, return the one with lower value.
 *
 * if either is NULL, return the other.
 * of both are NULL, return NULL.
 */
static inline wtimer_t *_compare(wtimer_t *a, wtimer_t *b)
{
    if (a && b) {
        return a->target <= b->target ? a : b;
    } else {
        return a ? a : b;
    }
}

/**
 * @brief merge two timer lists, return head of new list
 */
static wtimer_t *_merge_lists(wtimer_t *head_a, wtimer_t *head_b)
{
    wtimer_t *result_head = _compare(head_a, head_b);
    wtimer_t *pos = result_head;

    while(1) {
        head_a = head_a->next;
        head_b = head_b->next;
        if (!head_a) {
            pos->next = head_b;
            break;
        }
        if (!head_b) {
            pos->next = head_a;
            break;
        }

        pos->next = _compare(head_a, head_b);
        pos = pos->next;
    }

    return result_head;
}

/**
 * @brief parse long timers list and copy those that will expire in the current
 *        short timer period
 */
static void _select_long_timers(void)
{
    wtimer_t *select_list_start = long_list_head;
    wtimer_t *select_list_last = NULL;

    /* advance long_list head so it points to the first timer of the next (not
     * just started) "long timer period" */
    while (long_list_head) {
        if ((long_list_head->long_target <= _long_cnt) && _this_high_period(long_list_head->target)) {
            select_list_last = long_list_head;
            long_list_head = long_list_head->next;
        }
        else {
            /* remaining long_list timers belong to later long periods */
            break;
        }
    }

    /* cut the "selected long timer list" at the end */
    if (select_list_last) {
        select_list_last->next = NULL;
    }

    /* merge "current timer list" and "selected long timer list" */
    if (timer_list_head) {
        if (select_list_last) {
            /* both lists are non-empty. merge. */
            timer_list_head = _merge_lists(timer_list_head, select_list_start);
        }
        else {
            /* "selected long timer list" is empty, nothing to do */
        }
    }
    else { /* current timer list is empty */
        if (select_list_last) {
            /* there's no current timer list, but a non-empty "selected long
             * timer list".  So just use that list as the new current timer
             * list.*/
            timer_list_head = select_list_start;
        }
    }
}

/**
 * @brief handle low-level timer overflow, advance to next short timer period
 */
static void _next_period(void)
{
#if WTIMER_MASK
    /* advance <32bit mask register */
    _high_cnt += ~WTIMER_MASK + 1;
    if (! _high_cnt) {
        /* high_cnt overflowed, so advance >32bit counter */
        _long_cnt++;
    }
#else
    /* advance >32bit counter */
    _long_cnt++;
#endif

    /* swap overflow list to current timer list */
    timer_list_head = overflow_list_head;
    overflow_list_head = NULL;

    _select_long_timers();

}

/**
 * @brief main wtimer callback function
 */
static void _timer_callback(void)
{
    uint32_t next;
    uint32_t reference;

    DEBUG("_timer_callback() now=%u (%u)pleft=%u\n", (unsigned)wtimer_now(),
            (unsigned)_mask(wtimer_now()), (unsigned) _mask(0xffffffff-wtimer_now()));

    if (!timer_list_head) {
        DEBUG("_timer_callback(): tick\n");
        /* there's no timer for this timer period,
         * so this was a timer overflow callback.
         *
         * In this case, we advance to the next timer period.
         */
        _next_period();

        reference = 0;

        /* make sure the timer counter also arrived
         * in the next timer period */
        while (_wtimer_now() == _mask(0xFFFFFFFF));
    }
    else {
        /* we ended up in _timer_callback and there is
         * a timer waiting.
         */
        /* set our period reference to that timer's target time. */
        reference = _mask(timer_list_head->target);
    }

overflow:
    /* check if next timers are close to expiring */
    while (timer_list_head && _time_left(_mask(timer_list_head->target), reference) < WTIMER_ISR_BACKOFF) {
        /* make sure we don't fire too early */
        while (_time_left(_mask(timer_list_head->target), 0));

        _shoot(timer_list_head);

        /* advance to next timer in list */
        timer_list_head = timer_list_head->next;
    }

    /* possibly executing all callbacks took enough
     * time to overflow.  In that case we advance to
     * next timer period and check again for expired
     * timers.*/
    if (reference > _wtimer_now()) {
        DEBUG("_timer_callback: overflowed while executing callbacks. %i\n", timer_list_head != 0);
        _next_period();
        reference = 0;
        goto overflow;
    }

    if (timer_list_head) {
        /* schedule callback on next timer target time */
        next = timer_list_head->target - WTIMER_OVERHEAD;
    }
    else {
        /* there's no timer planned for this timer period */
        /* schedule callback on next overflow */
        next = _mask(0xFFFFFFFF);
        uint32_t now = _wtimer_now();

        /* check for overflow again */
        if (now < reference) {
            _next_period();
            reference = 0;
            goto overflow;
        } else {
            /* check if the end of this period is very soon */
            if (_mask(now + WTIMER_ISR_BACKOFF) < now) {
                /* spin until next period, then advance */
                while (_wtimer_now() > now);
                _next_period();
                reference = 0;
                goto overflow;
            }
        }
    }

    /* set low level timer */
    _lltimer_set(next);
}
