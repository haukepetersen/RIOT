

#include <limits.h>
#include <string.h>

#include "log.h"
#include "ztimer.h"

#include "hilo.h"

#define MS_PER_HOUR         (3600 * 1000UL)

void hilo_init(hilo_t *hilo)
{
    memset(hilo, 0, sizeof(hilo_t));
    // hilo->on_pulse = cb;
    hilo->maxhi = INT_MIN;
    hilo->minlo = INT_MAX;
}

void hilo_update(hilo_t *hilo, int val)
{
    hilo->minlo = (val < hilo->minlo) ? val : hilo->minlo;
    hilo->maxhi = (val > hilo->maxhi) ? val : hilo->maxhi;
    int off = ((hilo->maxhi - hilo->minlo) / HILO_OFFSET);
    hilo->hi = hilo->maxhi - off;
    hilo->lo = hilo->minlo + off;
    LOG_INFO("v:%6i hi:%6i lo:%6i state:%i\n", val, hilo->hi, hilo->lo, (int)hilo->state);
}

void hilo_calc_avg(hilo_t *hilo)
{
    uint32_t now = ztimer_now(ZTIMER_MSEC);

    uint16_t cnt = hilo->cnt - hilo->last_cnt;
    hilo->cnt_per_h = (uint16_t)(((uint32_t)MS_PER_HOUR * cnt) / (now - hilo->last_pulse));
    hilo->last_pulse = now;
    hilo->last_cnt = hilo->cnt;
}

void hilo_sample(hilo_t *hilo, int val)
{
    if ((val < hilo->lo) && (hilo->state == 0)) {
        hilo->state = 1;
        hilo->minlo = val;
        hilo->cnt ++;
    }
    else if ((val > hilo->hi) && (hilo->state == 1)) {
        hilo->state = 0;
        hilo->maxhi = val;
    }

    hilo_update(hilo, val);
}
