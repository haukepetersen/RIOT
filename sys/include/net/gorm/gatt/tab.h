/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_gatt_tab Gorm's GATT Table
 * @ingroup     net_gorm_gatt
 * @brief       Gorm's GATT table implementation
 * @{
 *
 * @file
 * @brief       Gorm's GATT table interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_GATT_TAB_H
#define GORM_GATT_TAB_H

#include "net/gorm/gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GORM_GATT_TAB_FIRST_SIG_HANDLE      (0x0100)
#define GORM_GATT_TAB_FIRST_CUSTOM_HANDLE   (0x8100)
#define GORM_GATT_TAB_CHAR_VAL_MASK         (0x0008)
/**
 * @brief   GATT table iterator
 */
typedef struct {
    const gorm_gatt_service_t *s; /**< pinter to a service */
    const gorm_gatt_char_t *c;  /**< pointer to a characteristic */
    const gorm_gatt_desc_t *d;  /**< pointer to a descriptor */
    uint16_t handle;            /**< corresponding/current handle */
} gorm_gatt_tab_iter_t;



void gorm_gatt_tab_reg_service(gorm_gatt_service_t *service);

/**
 * @brief   Get the entry corresponding to the the given handle
 *
 * @param[in,out] iter      Resulting entry, which @p iter.handle holding the
 *                          handle to look for
 */
void gorm_gatt_tab_get(gorm_gatt_tab_iter_t *iter);
void gorm_gatt_tab_get_next(gorm_gatt_tab_iter_t *iter, uint16_t end_handle);

gorm_gatt_service_t *gorm_gatt_tab_service_by_uuid(gorm_uuid_t *type,
                                                   uint16_t start, uint16_t end);

uint16_t gorm_gatt_tab_service_next_handle(uint16_t last);
uint16_t gorm_gatt_tab_service_last_handle(uint16_t handle);
uint16_t gorm_gatt_tab_char_val_handle(uint16_t char_handle);
uint16_t gorm_gatt_tab_char_closest_handle(uint16_t start);
uint16_t gorm_gatt_tab_char_next_handle(uint16_t last);
uint16_t gorm_gatt_tab_desc_closest_handle(uint16_t handle);
uint16_t gorm_gatt_tab_desc_next_handle(uint16_t last);


static inline int gorm_gatt_tab_is_service(const gorm_gatt_tab_iter_t *iter)
{
    return ((iter->s != NULL) && (iter->c == NULL));
}

static inline int gorm_gatt_tab_is_char_decl(const gorm_gatt_tab_iter_t *iter)
{
    return ((iter->c != NULL) && (iter->d == NULL) &&
            !(iter->handle & GORM_GATT_TAB_CHAR_VAL_MASK));
}

static inline int gorm_gatt_tab_is_char_val(const gorm_gatt_tab_iter_t *iter)
{
    return ((iter->c != NULL) && (iter->d == NULL) &&
            (iter->handle & GORM_GATT_TAB_CHAR_VAL_MASK));
}

static inline int gorm_gatt_tab_is_decl(const gorm_gatt_tab_iter_t *iter)
{
    return (iter->d != NULL);
}

#ifdef __cplusplus
}
#endif

#endif /* GORM_GATT_TAB_H */
/** @} */