
#define TIM_BURST_CHAN          (0)

/**
 * @brief   List for keeping active timers
 */
static tim_burst_t *timers;

static void local_cb(void *arg, int channel)
{
    /* remove currently active timer from list */
    tim_burst_t *tim = timers;
    timers = tim->next;

    /* trigger callback */
    tim->cb(tim->arg);

    /* set next timer */
    if (timers) {
        /* TODO: some magic for handling overflows etc */
    }
}

void tim_burst_init(void)
{
    /* initialize the low-level timer */
    timer_init(TIM_BURST_TIMER, TIM_BURST_HZ, local_cb, NULL);
}
