
#include <stdio.h>

#include "irq.h"
#include "assert.h"
#include "tim.h"
#include "tim/plex.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"


static volatile uint32_t last = 0;

#if ENABLE_DEBUG
static void dump(tim_base_t **queue, const char *title)
{
    tim_base_t *head = *queue;
    int i = 0;

    DEBUG("List after %s\n", title);
    while (head) {
        DEBUG("%02i - t: %p to: %i\n", i++, (void *)head, (int)head->t0);
        head = head->next;
    }
}
#endif

static inline void compensate(const tim_llt_t *plex, tim_base_t *t, uint32_t last)
{
    uint32_t diff = ((plex->now() - last) & plex->mask);
    t->t0 = (t->t0 > diff) ? (t->t0 - diff) : 0;
}

void tim_plex_set_next(const tim_llt_t *plex, tim_base_t **queue, uint32_t ref)
{
    /* TODO: update head time again and trigger all timers that to to 0 */
    while (*queue && ((*queue)->t0 < TIM_BACKOFF)) {
        /* dequeue and execute callback */
        tim_base_t *t = *queue;
        *queue = t->next;
        t->action(t->arg);
    }
    if (*queue) {
        compensate(plex, *queue, ref);
        plex->set((*queue)->t0);        /* TODO: handle underflows -> use setnext() */
    }
}

void tim_plex_set(const tim_llt_t *plex, tim_base_t **queue, tim_base_t *t)
{
    assert(plex && queue && t);

    unsigned is = irq_disable();    /* @todo: possibly make crit section samller? */

    plex->clr();
    uint32_t now = plex->now();

    /* get pointer to head of list */
    tim_base_t *head = *queue;

    /* insert if no timer is set */
    if (!head) {
        DEBUG("plex: INSERT HEAD\n");
        t->next = NULL;
        *queue = t;
    }
    else {
        DEBUG("plex: INSERT MID\n");
        /* update existing list head */
        compensate(plex, head, last);

        /* is new timer smaller than first in list -> insert first */
        if (t->t0 < head->t0) {
            head->t0 -= t->t0;
            t->next = head;
            *queue = t;
        }
        /* else we need to find correct position in list */
        else {
            t->t0 -= head->t0;
            while (head->next && (t->t0 >= head->next->t0)) {
                head = head->next;
                t->t0 -= head->t0;
            }
            t->next = head->next;
            if (t->next) {
                t->next->t0 -= t->t0;
            }
            head->next = t;
        }
    }
    last = now;

    tim_plex_set_next(plex, queue, now);

    irq_restore(is);

    // dump(queue, "SET");
}

void tim_plex_rm(const tim_llt_t *plex, tim_base_t **queue, tim_base_t *t)
{
    assert(plex && queue && t);

    unsigned is = irq_disable();
    tim_base_t *head = *queue;

    if (!head) {
        irq_restore(is);
        return;
    }

    if (head == t) {
        plex->clr();
        *queue = head->next;
        if (*queue) {
            plex->set((*queue)->t0);
        }
    }
    else {
        while (head->next && (head->next != t)) {}
        if (head->next == t) {
            head->next = t->next;
        }
    }
    irq_restore(is);

    // dump(queue, "RM");
}
