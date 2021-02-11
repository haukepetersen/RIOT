

#include <limits.h>
#include <string.h>

#include "log.h"
#include "ztimer.h"

#include "hilo.h"

#define MS_PER_HOUR         (3600 * 1000U)

void hilo_init(hilo_t *hilo)
// void hilo_init(hilo_t *hilo, hilo_cb_t cb)
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

void hilo_sample(hilo_t *hilo, int val)
{
    if ((val < hilo->lo) && (hilo->state == 0)) {
        hilo->state = 1;
        hilo->minlo = val;

        // hilo->on_pulse();
        /* on pulse */
        uint32_t now = ztimer_now(ZTIMER_MSEC);
        hilo->cnt ++;
        hilo->cnt_per_h = (uint16_t)(MS_PER_HOUR / (now - hilo->last_pulse));
        hilo->last_pulse = now;

    }
    else if ((val > hilo->hi) && (hilo->state == 1)) {
        hilo->state = 0;
        hilo->maxhi = val;
    }

    hilo_update(hilo, val);
}
