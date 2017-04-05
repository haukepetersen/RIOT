
#include "assert.h"
#include "mutex.h"
#include "tim.h"
#include "tim/plex.h"

#include "periph/timer.h"
#include "periph/rtt.h"


static void tim_rtt_cb(void *arg);
static uint32_t now_timer(void);
static void set_timer(uint32_t to);
static void set_rtt(uint32_t to);
static void clr_timer(void);

typedef struct {
    tim_base_t base;
    mutex_t lock;
} tim_lock_t;

static tim_base_t *burst_queue;
static tim_base_t *cont_queue;

static const tim_llt_t burst_llt = {
    .now  = &now_timer,
    .set  = &set_timer,
    .clr  = &clr_timer,
    .mask = TIM_TIMER_MASK,
    .num  = 0
};

static const tim_llt_t cont_llt = {
    .now  = &rtt_get_counter,
    .set  = &set_rtt,
    .clr  = &rtt_clear_alarm,
    .mask = RTT_MAX_VALUE,
    .num  = 0
};

static uint32_t now_timer(void)
{
    return (uint32_t)timer_read(TIM_TIMER_DEV);
}

static void set_timer(uint32_t to)
{
    // printf("setting TIMER to %i\n", (int)to);
    timer_set(TIM_TIMER_DEV, 0, (unsigned)to);
}

static void set_rtt(uint32_t to)
{
    // printf("setting RTT to %i\n", (int)to);
    rtt_set_alarm(rtt_get_counter() + to, tim_rtt_cb, NULL);
}

static void clr_timer(void)
{
    timer_clear(TIM_TIMER_DEV, 0);
}

static void trigger_unlock(void *arg)
{
    mutex_unlock((mutex_t *)arg);
}

static void trigger_msg(void *arg)
{
    tim_msg_t *t = (tim_msg_t *)t;
    msg_send(&t->msg, t->pid);
}

static void tim_rtt_cb(void *arg)
{
    (void)arg;

    // puts("TO: RTT");

    uint32_t now = rtt_get_counter();

    /* dequeue timer */
    tim_base_t *t = cont_queue;
    cont_queue = cont_queue->next;
    /* trigger timer action */
    t->action(t->arg);
    /* schedule next */
    tim_plex_set_next(&cont_llt, &cont_queue, now);

    // puts("TO: RTT done");
}

static void timer_cb(void *arg, int channel)
{
    (void)channel;
    (void)arg;

    // puts("TO: TIMER");

    uint32_t now = timer_read(TIM_TIMER_DEV);

    /* dequeue timer */
    tim_base_t *t = burst_queue;
    burst_queue = burst_queue->next;
    /* trigger action */
    t->action(t->arg);
    /* and schedule next */
    tim_plex_set_next(&burst_llt, &burst_queue, now);

    // puts("TO: TIMER done");
}

void tim_init(void)
{
    rtt_init();
    timer_init(TIM_TIMER_DEV, TIM_TIMER_FREQ, timer_cb, NULL);
}

void tim_sleep(tim_timer_t timer, uint32_t to)
{
    mutex_t lock = MUTEX_INIT_LOCKED;
    tim_base_t t = { NULL, to, 0, trigger_unlock, (void *)&lock };

    set(timer, &t);
    mutex_lock(&lock);
}

void tim_set_cb(tim_timer_t *timer, uint32_t to, tim_base_t *t, tim_func_t cb, void *arg)
{
    t->t0 = to;
    t->action = cb;
    t->arg = arg;
    tim_plex_set()
    tim_plex_set(timer, t);
}

void tim_set_msg(tim_timer_t *timer, uint32_t to, tim_msg_t *t, kernel_pid_t pid, msg_t *msg)
{
    t->base.t0 = to;
    t->base.action = trigger_msg;
    t->base.arg = (void *)t;
    t->pid = pid;
    t->msg = *msg;
    tim_plex_set(timer, (tim_base_t *)t);
}
