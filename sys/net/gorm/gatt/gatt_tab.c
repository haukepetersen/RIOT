/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gorm
 * @{
 *
 * @file
 * @brief       Gorm's GATT table implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#include <limits.h>
#include <string.h>

#include "assert.h"

#include "net/gorm/gap.h"
#include "net/gorm/gatt.h"
#include "net/gorm/gatt/tab.h"

#define ENABLE_DEBUG                (1)
#include "debug.h"

#define NONSIG_FLAG                 (0x8000)
#define SERVICE_NUM_MASK            (0x7f00)
#define SERVICE_MASK                (0xff00)
#define SERVICE_INC                 (0x0100)
#define CHAR_MASK                   (0x00f8)
#define CHAR_INC                    (0x0010)
#define CHAR_NUM_MASK               (0x00f0)
#define CHAR_NUM_POS                (4U)
#define CHAR_VAL_MASK               (0x0008)
#define DESC_MASK                   (0x0007)
#define DESC_INC                    (0x0001)

static gorm_gatt_service_t *_sig = NULL;
static gorm_gatt_service_t *_custom = NULL;
static mutex_t lock = MUTEX_INIT;

static gorm_gatt_service_t *_get_table_by_handle(uint16_t handle)
{
    return (handle & NONSIG_FLAG) ? _custom : _sig;
}

static gorm_gatt_service_t *_get_table_by_uuid(gorm_uuid_t *uuid)
{
    return (gorm_uuid_sig(uuid)) ? _sig : _custom;
}

static gorm_gatt_service_t *_get_service(uint16_t handle)
{
    gorm_gatt_service_t *s = _get_table_by_handle(handle);
    while (s && (s->handle != (handle & SERVICE_MASK))) {
        s = s->next;
    }
    return s;
}

static const gorm_gatt_char_t *_get_char(uint16_t handle,
                                         const gorm_gatt_service_t *s)
{
    if (handle & CHAR_MASK) {
        unsigned pos = (((handle & CHAR_NUM_MASK) >> CHAR_NUM_POS) - 1);
        if (pos < s->char_cnt) {
            return &s->chars[pos];
        }
    }
    return NULL;
}

static const gorm_gatt_desc_t *_get_desc(uint16_t handle,
                                         const gorm_gatt_char_t *c)
{
    if (handle & (DESC_MASK | CHAR_VAL_MASK)) {
        unsigned pos = ((handle & DESC_MASK) - 1);
        if (pos < c->desc_cnt) {
            return &c->desc[pos];
        }
    }
    return NULL;
}

void gorm_gatt_tab_reg_service(gorm_gatt_service_t *s)
{
    assert(s);

    /* prepare new entry */
    s->next = NULL;

    mutex_lock(&lock);
    /* add as first element into SIG table when empty */
    if (gorm_uuid_sig(&s->uuid) && (_sig == NULL)) {
        _sig = s;
        s->handle = GORM_GATT_TAB_FIRST_SIG_HANDLE;
    }
    /* add as first element in custom table when empty */
    else if (!gorm_uuid_sig(&s->uuid) && (_custom == NULL)) {
        _custom = s;
        s->handle = GORM_GATT_TAB_FIRST_CUSTOM_HANDLE;
    }
    /* else we get the last element of the fitting table and add the new service
     * there. */
    else {
        gorm_gatt_service_t *tab = _get_table_by_uuid(&s->uuid);
        while (tab->next != NULL) {
            tab = tab->next;
        }
        tab->next = s;
        s->handle = gorm_gatt_tab_service_next_handle(tab->handle);
    }
    mutex_unlock(&lock);
}

void gorm_gatt_tab_get(gorm_gatt_tab_iter_t *iter)
{
    assert(iter);
    iter->s = NULL;
    iter->c = NULL;
    iter->d = NULL;

    mutex_lock(&lock);
    iter->s = _get_service(iter->handle);
    if (iter->s == NULL) {
        goto end;
    }
    iter->c = _get_char(iter->handle, iter->s);
    if (iter->c == NULL) {
        goto end;
    }
    iter->d = _get_desc(iter->handle, iter->c);

end:
    mutex_unlock(&lock);
}

void gorm_gatt_tab_get_next(gorm_gatt_tab_iter_t *iter, uint16_t end_handle)
{
    assert(iter);

    mutex_lock(&lock);
    if (iter->d) {
        iter->handle = gorm_gatt_tab_desc_next_handle(iter->handle);
        if (iter->handle <= end_handle) {
            iter->d = _get_desc(iter->handle, iter->c);
        }
        else {
            iter->d = NULL;
        }
    }
    else if (iter->c) {
        iter->handle = gorm_gatt_tab_char_next_handle(iter->handle);
        if (iter->handle <= end_handle) {
            iter->c = _get_char(iter->handle, iter->s);
        }
        else {
            iter->c = NULL;
        }
    }
    else if (iter->s) {
        iter->s = iter->s->next;
        if (iter->s && (iter->s->handle <= end_handle)) {
            iter->handle = iter->s->handle;
        }
        else {
            iter->s = NULL;
        }
    }
    mutex_unlock(&lock);
}

gorm_gatt_service_t *gorm_gatt_tab_service_by_uuid(gorm_uuid_t *type,
                                                   uint16_t start, uint16_t end)
{
    assert(type);

    mutex_lock(&lock);
    gorm_gatt_service_t *s = _get_table_by_uuid(type);
    while (s && (s->handle < start)) {
        s = s->next;
    }
    while (s && (s->handle <= end)) {
        if (gorm_uuid_equal(&s->uuid, type)) {
            mutex_unlock(&lock);
            return s;
        }
    }
    mutex_unlock(&lock);
    return NULL;
}

uint16_t gorm_gatt_tab_service_next_handle(uint16_t last)
{
    if ((last & SERVICE_NUM_MASK) == SERVICE_NUM_MASK) {
        return UINT16_MAX;
    }
    return ((last + SERVICE_INC) & SERVICE_MASK);
}

uint16_t gorm_gatt_tab_service_last_handle(uint16_t handle)
{
    return (gorm_gatt_tab_service_next_handle(handle) - 1);
}

uint16_t gorm_gatt_tab_char_val_handle(uint16_t char_handle)
{
    return (char_handle | CHAR_VAL_MASK);
}

uint16_t gorm_gatt_tab_char_closest_handle(uint16_t start)
{
    if ((start & CHAR_NUM_MASK) && !(start & DESC_MASK)) {
        start -= 1;
    }
    return gorm_gatt_tab_char_next_handle(start);
}

uint16_t gorm_gatt_tab_char_next_handle(uint16_t last)
{
    return ((last + CHAR_INC) & (SERVICE_MASK | CHAR_NUM_MASK));
}

uint16_t gorm_gatt_tab_desc_closest_handle(uint16_t handle)
{
    if (!(handle & CHAR_VAL_MASK)) {
        handle |= CHAR_VAL_MASK;
    }
    if (handle & DESC_MASK) {
        handle -= 1;
    }
    return gorm_gatt_tab_desc_next_handle(handle);
}

uint16_t gorm_gatt_tab_desc_next_handle(uint16_t last)
{
    return (last + DESC_INC);
}
