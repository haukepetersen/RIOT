

#define TIM_BURST_TIMER         (TIMER_DEV(0))
#define TIM_BURST_RES           (0xffffffff)
#define TIM_BURST_BACKOFF       (30)
#define TIM_BURST_HZ            (1 * US_PER_SEC)

#define TIM_BURST_SHIFT         (0)

typedef struct tim_burst {
    struct tim_burst *next;
    uint32_t timeout;
    tim_cb_t cb;
    void *arg;
} tim_burst_t;


static inline void tim_burst_assert_res(uint32_t res)
{
    assert(!(res - (res & TIM_BURST_RES)));
}

static inline void tim_burst_assert_range(uint32_t range)
{
    assert()
}

void tim_burst_init(void);

void tim_burst_request(void);
void tim_burst_release(void);


uint32_t tim_burst_now_us(void);
uint32_t tim_burst_since(uint32_t pit);

void tim_burst_usleep(uint32_t us);

void tim_burst_cb(tim_burst_t *tim, uint32_t timeout);
void tim_burst_msg(tim_burst_t *tim, msg_t *msg, uint32_t timeout);
void tim_burst_flag(tim_burst_t *tim, thread_t *thread, thread_flags_t mask, uint32_t timeout);
void tim_burst_wakeup(tim_burst_t *tim, kernel_pid_t pid, uint32_t timeout);

void tim_burst_remove(tim_burst_t *tim);
