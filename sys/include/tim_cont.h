

/**
 *
 * Considerations:
 *
 * - setting an interval of uint32_t ms -> 48 days should be sufficient?
 * - everything above this must be handled by sw timers externally
 *
 * Global time / Date:
 * - sw interrupt on overflow of underlying timer
 * - worst case: 16-bit rtt @ 32768khz -> overflow every 2 seconds
 *    BUT: 16-bit rtt unlikely, more likely to be at least 24-bit -> overflow every 8.5 min
 *
 *
 *
 *
 * @param  [description]
 * @return [description]
 */

#include "log.h"


#define TIM_CONT_RES        (0xffffffff)
#define TIM_CONT_SHIFT      (0)



static inline void tim_cont_assert_res(uint32_t freq)
{
    if (freq >= TIM_CONT_HZ) {
        LOG_ERROR("[tim_cont] requested frequency is not delivered\n");
        assert(freq >= TIM_CONT_HZ);
    }
}



void tim_cont_init(void);



uint32_t tim_cont_get_sec(void);
uint32_t tim_cont_get_ms(void);


void tim_cont_cb(tim_t *tim, uint32_t timeout)
