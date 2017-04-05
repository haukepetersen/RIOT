

#ifndef TIM_CONV_H
#define TIM_CONV_H

#include "tim.h"

#ifdef __cplusplus
extern "C" {
#endif


static inline uint32_t tim_conf_s(uint32_t tps, uint32_t to)
{
    return ((uint64_t)tps * to);
}

static inline uint32_t tim_conv_ms(uint32_t tps, uint32_t to)
{
    return ((uint64_t)tps * to) / 1000;
}

static inline uint32_t tim_conv_us(uint32_t tps, uint32_t to)
{
    return ((uint64_t)tps * to) / 1000000;
}

#ifdef __cplusplus
}
#endif

#endif /* TIM_CONV_H */
/** @} */
