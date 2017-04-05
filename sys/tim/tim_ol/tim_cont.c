

static volatile uint32_t swtick;

static tim_t *timers;

static inline uint32_t diff(tim_t *a, tim_t *b)
{
    return (a->timeout - b->timeout) & RTT_MASK;
}

static void cb_overflow(void)
{
    ++swtick;
    if (soft && (soft->timeout == swtick)) {
        reschedule(soft);
    }
}

static void cb_unlock(void *arg)
{
    mutex_t *m = (mutex_t *)arg;
    mutex_unlock(m);
}


static void set_tim(tim_t *tim)
{
    rtt_clear();

    tim_t head = tim;
    while (tim && tim->duration)
    if (!tims) {
        tims = tim;
    }
    else {
        tim_t *head = tim;
    }
}

uint32_t tim_cont_get_sec(void)
{
    uint32_t now = rtt_read();
}


void tim_cont_get_ms(void)
{

}

void tim_cont_get_sec(void)
{

}

void tim_cont_sleep_ms(uint32_t duration)
{
    /* prepare mutex */
    mutex_t m = MUTEX_INIT_LOCKED;
    tim_t tim;

    /* set timer */
    tim_cont_cb(&tim, cb_unlock, &m, duration);

    /* and wait until mutex is released */
    mutex_lock(m);
}

void tim_cont_cb(tim_t *tim, tim_cb_t cb, void *arg, uint32_t offset)
{
    assert(tim && cb);

    /* calculate absolute RTT value */
    uint32_t now = rtt_read();
    uint64_t tmp = ((uint64_t)RTT_TICKS_PER_SEC * offset);
    uint32_t ticks = (uint32_t)(tmp / 1000);

    /* value in range? */
    assert(ticks <= RTT_MASK);

    /* below backoff threshold? */
    if (ticks < TIM_CONT_BACKOFF) {
        cb(arg);
        return;
    }

    /* prepare tim */
    tim->target = (now + ticks) & RTT_MASK;
    tim->cb = cb;
    tim->arg = arg;

    /* activate timer */
    set_tim(tim);
}
