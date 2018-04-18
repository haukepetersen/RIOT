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

#include "assert.h"

#include "net/gorm/gap.h"
#include "net/gorm/gatt.h"
#include "net/gorm/gatt/tab.h"

#define ENABLE_DEBUG                (1)
#include "debug.h"

#define FIRST_SIG_HANDLE            (0x0100)
#define FIRST_CUSTOM_HANDLE         (0x8100)

#define NONSIG_FLAG                 (0x8000)
#define SERVICE_NUM_MASK            (0x7f00)
#define SERVICE_MASK                (0xff00)
#define SERVICE_HANDLE_INC          (0x0100)
#define SERVICE_HANDLE_LAST         (0xff00)
#define CHAR_MASK                   (0x00f8)
#define CHAR_INC                    (0x0010)
#define CHAR_NUM_MASK               (0x00f0)
#define CHAR_NUM_POS                (4U)
#define CHAR_VAL_MASK               (0x0008)
#define DESC_MASK                   (0x0007)
#define DESC_INC                    (0x0001)
#define VAL_DESC_MASK               (0x000f)

static gorm_gatt_entry_t *_sig = NULL;
static gorm_gatt_entry_t *_custom = NULL;
static mutex_t lock = MUTEX_INIT;

/* good */
static gorm_gatt_entry_t *_get_table_by_handle(uint16_t handle)
{
    return (handle & NONSIG_FLAG) ? _custom : _sig;
}

static gorm_gatt_service_t *_get_table_by_uuid(gorm_uuit_t *uuid)
{
    return (gorm_uuid_sig(uuid)) ? _sig : _custom;
}

// /* REMOVE, use '_get_entry()' instead */
// static gorm_gatt_entry_t *_get_exact(uint16_t handle)
// {
//     gorm_gatt_entry_t *res = _get_table(handle);
//     DEBUG("get_exact: base is %p\n", res);
//     while (res && (res->handle != handle)) {
//         res = res->next;
//     }
//     return res;
// }

// static uint16_t _char_handle(uint16_t base, uint8_t num)
// {
//     return ((base & SERVICE_MASK) | (((num + 1) << CHAR_NUM_POS) & CHAR_NUM_MASK));
// }




/* good */
static unsigned _get_char_num(uint16_t handle)
{
    return ((handle & CHAR_NUM_MASK) >> CHAR_NUM_POS) - 1;
}

/* good */
static unsigned _get_desc_num(uint16_t handle)
{
    return ((handle & DESC_MASK) - 1);
}

/* good */
static uint16_t _inc_char_handle(uint16_t handle)
{
    return (handle + CHAR_INC);
}

/* good */
static uint16_t _inc_desc_handle(uint16_t handle)
{
    return (handle + DESC_INC);
}



/* does not check if the handle actually exists... */
static uint16_t _find_char_handle(uint16_t start_handle)
{
    if ((start_handle & CHAR_MASK) && !(start_handle & VAL_DESC_MASK)) {
        return start_handle;
    }
    uint8_t num = _get_char_num(start_handle) + 1;
    return _char_handle(start_handle, num);
}

/* good */
static gorm_gatt_service_t *_get_service(uint16_t handle)
{
    gorm_gatt_service_t *s = _get_table_by_handle(handle);
    while (s && (s->handle != (handle & SERVICE_MASK))) {
        s = s->next;
    }
    return s;
}

/* good */
static const gorm_gatt_char_t *_get_char(uint16_t handle,
                                         const gorm_gatt_entry_t *e)
{
    if (handle & CHAR_MASK) {
        unsigned num = _get_char_num(handle);
        if (num < e->service->char_cnt) {
            return &e->service->chars[num];
        }
    }
    return NULL;
}

/* good */
static const gorm_gatt_desc_t *_get_desc(uint16_t handle,
                                         const gorm_gatt_char_t *c)
{
    if (handle & DESC_MASK) {
        unsigned num = _get_desc_num(handle);
        if (num < c->desc_cnt) {
            return &c->desc[num];
        }
    }
    return NULL;
}

/* good */
void gorm_gatt_tab_reg_service(gorm_gatt_service_t *s)
{
    assert(s);

    /* prepare new entry */
    s->next = NULL;

    mutex_lock(&lock);
    /* add as first element into SIG table when empty */
    if (gorm_uuid_sig(&s->uuid) && (_sig == NULL)) {
        _sig = s;
        s->handle = FIRST_SIG_HANDLE;
    }
    /* add as first element in custom table when empty */
    else if (!gorm_uuid_sig(&s->uuid) && (_custom == NULL)) {
        _custom = s;
        s->handle = FIRST_CUSTOM_HANDLE;
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

/* good */
void gorm_gatt_tab_get(gorm_gatt_tab_iter_t *iter, uint16_t handle)
{
    assert(iter);
    memset(iter, 0, sizeof(gorm_gatt_tab_iter_t));

    mutex_lock(&lock);
    iter->e = _get_service(iter->handle);
    if (iter->e == NULL) {
        goto end;
    }
    iter->c = _get_char(iter->handle, iter->e);
    if (iter->c == NULL) {
        goto end;
    }
    iter->d = _get_desc(iter->handle, iter->c);

end:
    mutex_unlock(&lock);
}

/* good */
void gorm_gatt_tab_get_next(gorm_gatt_tab_iter_t *iter)
{
    assert(iter);

    mutex_lock(&lock);
    if (iter->d) {
        iter->handle = _inc_desc_handle(iter->handle);
        iter->d = _get_desc(iter->handle, iter->c);
    }
    else if (iter->c) {
        iter->handle = _inc_char_handle(iter->handle);
        iter->c = _get_char(iter->handle, iter->e);
    }
    else if (iter->e) {
        iter->e = iter->e->next;
        iter->handle = (iter->e) ? iter->e->handle : UINT16_MAX;
    }
    mutex_unlock(&lock);
}

uint16_t gorm_gatt_tab_service_next_handle(uint16_t last)
{
    if ((last & SERVICE_NUM_MASK) == SERVICE_NUM_MASK) {
        return UINT16_MAX;
    }
    return ((last + SERVICE_HANDLE_INC) & SERVICE_MASK);
}

uint16_t gorm_gatt_tab_service_last_handle(uint16_t handle)
{
    return (gorm_gatt_tab_service_next_handle(handle) - 1);
}


static gorm_gatt_service_t *_find_service_by_uuid(gorm_uuid_t *type,
                                                  uint16_t start, uint16_t end)
{
    gorm_gatt_tab_iter_t iter;
    gorm_gatt_tab_get(&iter, start);

    while (iter.s && (iter.handle <= end)) {
        if (gorm_uuid_equal(&iter.s->uuid, type)) {
            return (gorm_gatt_service_t *)iter.s;
        }
    }

    return NULL;
}

gorm_gatt_service_t *gorm_gatt_tab_service_by_uuid(gorm_uuid_t *type,
                                                   uint16_t start, uint16_t end)
{
    assert(type);

    uint16_t handle = gorm_gatt_tab_service_next_handle(start - 1);
    gorm_gatt_service_t *s = _find_service_by_uuid(type, start, end);

    if (s == NULL && (!(handle & NONSIG_FLAG)) && (FIRST_CUSTOM_HANDLE <= end)) {
        s = _find_service_by_uuid(type, FIRST_CUSTOM_HANDLE, end);
    }

    return s;
}





gorm_gatt_entry_t *gorm_gatt_tab_find_service(uint16_t start_from)
{
    mutex_lock(&lock);
    gorm_gatt_entry_t *res = _get_table_by_handle(start_from);
    while (res && (res->next) && (res->handle < start_from)) {
        res = res->next;
    }
    mutex_unlock(&lock);

    return res;
}

gorm_gatt_entry_t *gorm_gatt_tab_get_service(uint16_t handle)
{
    DEBUG("get service: 0x%04x\n", (int)handle);

    mutex_lock(&lock);
    gorm_gatt_entry_t *res = _get_exact(handle & SERVICE_MASK);
    mutex_unlock(&lock);
    return res;
}

int gorm_gatt_tab_find_char(gorm_gatt_entry_t *entry, uint16_t start_handle)
{
    uint16_t handle = _find_char_handle(start_handle);
    uint16_t num = _get_char_num(handle);
    if (num < entry->service->char_cnt) {
        return (int)num;
    }
    return -1;
}

uint16_t gorm_gatt_tab_get_end_handle(const gorm_gatt_entry_t *entry)
{
    if (entry->next) {
        return (entry->next->handle - 1);
    }
    if (!(entry->handle & NONSIG_FLAG) && _custom) {
        return (_custom->handle - 1);
    }
    return UINT16_MAX;
}

uint16_t gorm_gatt_tab_get_char_handle(gorm_gatt_entry_t *entry, uint16_t num)
{
    return (entry->handle | ((num + 1) << CHAR_NUM_POS));
}

uint16_t gorm_gatt_tab_get_val_handle(gorm_gatt_entry_t *entry, uint16_t num)
{
    return (entry->handle | ((num + 1) << CHAR_NUM_POS) | CHAR_VAL_MASK);
}












/* TODO: remove or move to debug file or similar */
#include <stdio.h>
static void _print_head(uint16_t handle, const char *type)
{
    printf("| 0x%04x | %7s | ", (int)handle, type);
}

static void _print_uuid(const gorm_uuid_t *uuid)
{
    printf("UUID: ");
    if (gorm_uuid_sig(uuid)) {
        printf("0x%04x", (int)uuid->uuid16);
    }
    else {
        printf("UUID: %04x-%04x-%08x-%08x-%08x",
               (int)uuid->base->raw[0], (int)uuid->uuid16,
               (int)uuid->base->u32[1], (int)uuid->base->u32[2],
               (int)uuid->base->u32[3]);
    }
}

static void _print_table(gorm_gatt_entry_t *entry)
{
    while (entry) {
        _print_head(entry->handle, "SERVICE");
        _print_uuid(&entry->service->uuid);
        puts("");

        for (uint16_t i = 0; i < entry->service->char_cnt; i++) {
            uint16_t h = _char_handle(entry->handle, i);
            const gorm_gatt_char_t *c = &entry->service->chars[i];
            _print_head(h, "CHARACT");
            _print_uuid(&c->type);
            puts("");
            _print_head((h | CHAR_VAL_MASK), "C_VALUE");
            printf("properties: 0x%02x\n", (int)c->perm);
        }

        puts("");

        entry = entry->next;
    }
}

void gorm_gatt_tab_print(void)
{

    printf("| handle | type    | value\n");
    _print_table(_sig);
    _print_table(_custom);
    printf("|------------------|-------\n");
}
