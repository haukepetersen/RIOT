

#ifndef TIM_PLEX_H
#define TIM_PLEX_H

#include "tim.h"

#ifdef __cplusplus
extern "C" {
#endif


void tim_plex_set_next(const tim_llt_t *plex, tim_base_t **queue, uint32_t ref);


void tim_plex_set(const tim_llt_t *plex, tim_base_t **queue, tim_base_t *t);

void tim_plex_rm(const tim_llt_t *plex, tim_base_t **queue, tim_base_t *t);


#ifdef __cplusplus
}
#endif

#endif /* TIM_PLEX_H */
/** @} */
