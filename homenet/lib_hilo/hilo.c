

#include <limits.h>
#include <string.h>

#include "log.h"
#include "hilo.h"

void hilo_init(hilo_t *hilo, hilo_cb_t cb)
{
    memset(hilo, 0, sizeof(hilo_t));
    hilo->on_pulse = cb;
    hilo->minlo = UINT8_MAX;
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

        hilo->on_pulse();
    }
    else if ((val > hilo->hi) && (hilo->state == 1)) {
        hilo->state = 0;
        hilo->maxhi = val;
    }

    hilo_update(hilo, val);
}
