/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_gatt_tab GATT Table for Gorm
 * @ingroup     net_gorm_gatt
 * @brief       Gorm's GATT table implementation
 *
 * # About
 *
 * This submodule manages the a table with entries for the GATT server.
 *
 * # Allocation of Handles
 *
 * The ATT protocol defines that each unique attribute can be addressed using a
 * 16-bit handle. Allocating these handles is up to the implementation.
 *
 * Gorm is using a custom scheme for allocating these handles, enabling the
 * implementation to deduct all handles of attributes included in a service from
 * the service's base handle.
 *
 * For this, we simply assign the certain parts of the 16-bit 'handle space' to
 * specific meaning:
 *
 * 0bTSSSSSSSCCCCVDDD
 *      bit15: 0 -> service with 16-bit UUID, 1 -> service with 128-bit UUID
 * bit14-bit8: incremental ID for each service added, starting with 1
 *  bit4-bit7: incremental ID for each characteristic of a service
 *       bit3: 0 -> characteristic declaration, 1 -> characteristic value
 *  bit2-bit0: incremental ID for each descriptor of a characteristic
 *
 * Examples:
 * 0b1000001000000000 -> service #2 with 128-bit UUID
 * 0b0000000100111000 -> characteristic value of char #3 of service #1
 * 0b1000100000011010 -> descriptor #2 of char #1 of service #8
 * 0x1000000100001000 -> invalid
 * 0x1000000100010001 -> invalid
 *
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

/**
 * @name    Selected bit-masks for getting information from handles
 * @{
 */
#define GORM_GATT_TAB_FIRST_SIG_HANDLE      (0x0100)
#define GORM_GATT_TAB_FIRST_CUSTOM_HANDLE   (0x8100)
#define GORM_GATT_TAB_CHAR_VAL_MASK         (0x0008)
/** @} */

/**
 * @brief   GATT table iterator
 */
typedef struct {
    const gorm_gatt_service_t *s;   /**< pinter to a service */
    const gorm_gatt_char_t *c;      /**< pointer to a characteristic */
    const gorm_gatt_desc_t *d;      /**< pointer to a descriptor */
    uint16_t handle;                /**< corresponding/current handle */
} gorm_gatt_tab_iter_t;

/**
 * @brief   Add a new service to the GATT table
 *
 * @param[in] service       service to register
 */
void gorm_gatt_tab_reg_service(gorm_gatt_service_t *service);

/**
 * @brief   Get the entry corresponding to the the given handle
 *
 * @param[in,out] iter      resulting entry, which @p iter.handle holding the
 *                          handle to look for
 */
void gorm_gatt_tab_get(gorm_gatt_tab_iter_t *iter);

/**
 * @brief   Get the next entry of the same type from the table
 *
 * The type of current entry is read from @p iter, it can be the next service,
 * the next characteristic, or the next descriptor.
 *
 * @param[in,out] iter      holds the last entry found and will point to the
 *                          next entry in the list
 * @param[in] end_handle    only return the next element if its handle is
 *                          smaller or equal to this value
 */
void gorm_gatt_tab_get_next(gorm_gatt_tab_iter_t *iter, uint16_t end_handle);

/**
 * @brief   Find the next service with the given UUID and fitting handle
 *
 * @param[in] type          UUID to look for
 * @param[in] start         only look for services with a handle >= this value
 * @param[in] end           only look for services with a handle <= this value
 *
 * @return  pointer to the next fitting service
 * @return  NULL if no suitable service was found
 */
gorm_gatt_service_t *gorm_gatt_tab_service_by_uuid(const gorm_uuid_t *type,
                                                   uint16_t start,
                                                   uint16_t end);

/**
 * @brief   Generate the next service handle from the given handle
 *
 * @param[in] last          service handle
 *
 * @return  the next service handle following @p last
 * @return  UINT16_MAX if
 */
uint16_t gorm_gatt_tab_service_next_handle(uint16_t last);

/**
 * @brief   Generate the theoretical last handle usable by the given service
 *
 * @param[in] handle        any handle
 *
 * @return  last theoretical handle usable by the given service
 */
uint16_t gorm_gatt_tab_service_last_handle(uint16_t handle);

/**
 * @brief   Generate the characteristic value handle from the given handle
 *
 * @param[in] char_handle   characteristic declaration handle
 *
 * @return  handle with the characteristic value flag set
 */
uint16_t gorm_gatt_tab_char_val_handle(uint16_t char_handle);

/**
 * @brief   Generate the closest fitting characteristic declaration handle
 *
 * If the given handle is already a characteristic declaration handle, it is
 * simply returned. Else the handle for the next characteristic declaration in
 * the list is returned.
 *
 * @param[in] start        any handle
 *
 * @return  handle of the closest characteristic declaration
 */
uint16_t gorm_gatt_tab_char_closest_handle(uint16_t start);

/**
 * @brief   Generate the handle for the next characteristic declaration
 *
 * This function simply increments the given characteristic declaration handle
 * by one so it points to the next characteristic declaration in the list.
 *
 * @param[in] last          characteristic declaration handle
 *
 * @return  handle pointing to the next characteristic declaration in the list
 */
uint16_t gorm_gatt_tab_char_next_handle(uint16_t last);

/**
 * @brief   Generate the closest fitting descriptor handle
 *
 * @param[in] handle        any handle
 *
 * @return  handle pointing the the closest fitting descriptor
 */
uint16_t gorm_gatt_tab_desc_closest_handle(uint16_t handle);

/**
 * @brief   Generate handle for the next descriptor in the list
 *
 * @param[in] last          characteristic descriptor handle
 *
 * @return  handle pointing to descriptor after the given one
 */
uint16_t gorm_gatt_tab_desc_next_handle(uint16_t last);

/**
 * @brief   Test if given iterator points to a service
 *
 * @param[in] iter          iterator pointing to any GATT table item
 *
 * @return  0 if not a service
 * @return  >0 if a service
 */
static inline int gorm_gatt_tab_is_service(const gorm_gatt_tab_iter_t *iter)
{
    return ((iter->s != NULL) && (iter->c == NULL));
}

/**
 * @brief   Test if given iterator points to a characteristic declaration
 *
 * @param[in] iter          iterator pointing to any GATT table item
 *
 * @return  0 if not a characteristic declaration
 * @return  >0 if a characteristic declaration
 */
static inline int gorm_gatt_tab_is_char_decl(const gorm_gatt_tab_iter_t *iter)
{
    return ((iter->c != NULL) && (iter->d == NULL) &&
            !(iter->handle & GORM_GATT_TAB_CHAR_VAL_MASK));
}

/**
 * @brief   Test if the given iterator points to a characteristic value
 *
 * @param[in] iter          iterator pointing to any GATT table item
 *
 * @return  0 if not a characteristic value
 * @return  >0 if a characteristic value
 */
static inline int gorm_gatt_tab_is_char_val(const gorm_gatt_tab_iter_t *iter)
{
    return ((iter->c != NULL) && (iter->d == NULL) &&
            (iter->handle & GORM_GATT_TAB_CHAR_VAL_MASK));
}

/**
 * @brief   Test if the given iterator points to a characteristic descriptor
 *
 * @param[in] iter          iterator pointing to any GATT table item
 *
 * @return  0 if not a characteristic descriptor
 * @return  >0 if a characteristic descriptor
 */
static inline int gorm_gatt_tab_is_desc(const gorm_gatt_tab_iter_t *iter)
{
    return (iter->d != NULL);
}

#ifdef __cplusplus
}
#endif

#endif /* GORM_GATT_TAB_H */
/** @} */
