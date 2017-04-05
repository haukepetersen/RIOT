

static volatile uint32_t swtick;

static tim_t *timers;
static tim_t *soft;

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


static void set(tim_t *tim)
{
    if (tim)
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
    /* make sure we are in range */
    ...

    /* prepare mutex */
    mutex_t m = MUTEX_INIT;
    mutex_lock(m);

    /* prepare timeout value */
    uint32_t now = rtt_read();
    /* TODO: duration to ticks */
    duration += now;

    tim_t tim  = { NULL, duration, cb_unlock, &m };
    set_tim(&tim);

    mutex_lock(m);
}

void tim_cont_cb(tim_t *tim, tim_cb_t cb, void *arg, uint32_t offset)
{
    /* set to 250 ms */
    uint64_t tmp = ((uint64_t)RTT_TICKS_PER_SEC * offset);
    uint32_t target = (uint32_t)(tmp / 1000);

    /* value in range? */
    assert(!(target & (~RTT_WIDTH)));




    uint32_t now = rtt_read();
    uint32_t soft = offset / TICKS_PER_SEC;


}

#define TICKS_PER_SEC       32768
#define TICKS_PER_SEC       100
