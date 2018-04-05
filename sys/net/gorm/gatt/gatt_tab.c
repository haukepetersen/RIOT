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

#define HANDLE_GAP                  (0x0100)
#define HANDLE_ATT                  (0x0200)
#define FIRST_CUSTOM_HANDLE         (0x8100)

#define NONSIG_FLAG                 (0x8000)
#define SERVICE_MASK                (0xff00)
#define CHAR_MASK                   (0x00f8)
#define CHAR_INC                    (0x0010)
#define CHAR_NUM_MASK               (0x00f0)
#define CHAR_NUM_POS                (4U)
#define CHAR_VAL_MASK               (0x0008)
#define DESC_MASK                   (0x0007)
#define DESC_INC                    (0x0001)
#define VAL_DESC_MASK               (0x000f)

static gorm_gatt_entry_t gap_entry;
static gorm_gatt_entry_t att_entry;
static gorm_gatt_entry_t *customs = NULL;

static mutex_t lock = MUTEX_INIT;

/* forward declarations of callbacks for mandatory characteristics */
static size_t _on_gap_name(const gorm_gatt_char_t *characteristic,
                           uint8_t method, uint8_t *buf, size_t buf_len);
static size_t _on_gap_appearance(const gorm_gatt_char_t *characteristic,
                                 uint8_t method, uint8_t *buf, size_t buf_len);
static size_t _on_gap_con_param(const gorm_gatt_char_t *characteristic,
                                uint8_t method, uint8_t *buf, size_t buf_len);


/* declare mandatory services */
static const gorm_gatt_service_t gap_service = {
    .uuid  = GORM_UUID(GORM_UUID_GAP, NULL),
    .char_cnt = 3,
    .chars = (gorm_gatt_char_t[]){
        {
            .cb   = _on_gap_name,
            .type = GORM_UUID(GORM_UUID_DEVICE_NAME, NULL),
            .perm = BLE_ATT_READ,
        },
        {
            .cb   = _on_gap_appearance,
            .type = GORM_UUID(GORM_UUID_APPEARANCE, NULL),
            .perm = BLE_ATT_READ,
        },
        {
            .cb   = _on_gap_con_param,
            .type = GORM_UUID(GORM_UUID_PREF_CON_PARAM, NULL),
            .perm = BLE_ATT_READ,
        },
    },
};

static const gorm_gatt_service_t att_service = {
    .uuid  = GORM_UUID(GORM_UUID_ATT, NULL),
    .char_cnt = 0,
    .chars = NULL,
};

/* good */
static gorm_gatt_entry_t *_get_table(uint16_t handle)
{
    return (handle & NONSIG_FLAG) ? customs : &gap_entry;
}

/* REMOVE, use '_get_entry()' instead */
static gorm_gatt_entry_t *_get_exact(uint16_t handle)
{
    gorm_gatt_entry_t *res = _get_table(handle);
    DEBUG("get_exact: base is %p\n", res);
    while (res && (res->handle != handle)) {
        res = res->next;
    }
    return res;
}

static gorm_gatt_entry_t *_get_last(gorm_gatt_entry_t *start)
{
    assert(start);

    while (start->next != NULL) {
        start = start->next;
    }
    return start;
}

static uint16_t _next_service_handle(uint16_t last)
{
    return (((last >> 8) + 1) << 8);
}

static uint16_t _char_handle(uint16_t base, uint8_t num)
{
    return ((base & SERVICE_MASK) | (((num + 1) << CHAR_NUM_POS) & CHAR_NUM_MASK));
}




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
static gorm_gatt_entry_t *_get_entry(uint16_t handle)
{
    gorm_gatt_entry_t *e = _get_table(handle);
    while (e && (e->handle != (handle & SERVICE_MASK))) {
        e = e->next;
    }
    return e;
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

static size_t _on_gap_name(const gorm_gatt_char_t *characteristic,
                           uint8_t method, uint8_t *buf, size_t buf_len)
{
    (void)characteristic;
    /* TODO: check method */
    (void)method;

    DEBUG("READ READ READ: _on_gap_name()!\n");

    return gorm_gap_copy_name(buf, buf_len);
}

static size_t _on_gap_appearance(const gorm_gatt_char_t *characteristic,
                                 uint8_t method, uint8_t *buf, size_t buf_len)
{
    (void)characteristic;
    (void)method;
    (void)buf;
    (void)buf_len;

    DEBUG("READ READ READ: _on_gap_appearance()!\n");

    return 0;
}

static size_t _on_gap_con_param(const gorm_gatt_char_t *characteristic,
                                uint8_t method, uint8_t *buf, size_t buf_len)
{
    (void)characteristic;
    /* TODO: check this... */
    (void)method;

    DEBUG("READ READ READ: _on_gap_con_param()!\n");

    if (buf_len >= 2) {
        gorm_gap_copy_appearance(buf);
        return 2;
    }
    else {
        return 0;
    }
}

void gorm_gatt_tab_init(void)
{
    mutex_lock(&lock);
    gap_entry.next = &att_entry;
    gap_entry.service = &gap_service;
    gap_entry.handle = HANDLE_GAP;
    att_entry.next = NULL;
    att_entry.service = &att_service;
    att_entry.handle = HANDLE_ATT;
    mutex_unlock(&lock);

    DEBUG("[gorm_gatt_tab] initialization successful\n");
}

/* good */
void gorm_gatt_tab_reg_service(gorm_gatt_entry_t *entry,
                               const gorm_gatt_service_t *service)
{
    assert(entry && service);

    /* prepare new entry */
    entry->next = NULL;
    entry->service = service;

    /* find last entry of fitting table (16- vs 128- bit UUID...) */
    mutex_lock(&lock);
    gorm_gatt_entry_t *table = (gorm_uuid_sig(&service->uuid)) ?
                                                        &att_entry : customs;

    if (table == NULL) {
        customs = entry;
        entry->handle = FIRST_CUSTOM_HANDLE;
    }
    else {
        table = _get_last(table);
        table->next = entry;
        entry->handle = _next_service_handle(table->handle);
    }
    mutex_unlock(&lock);
}

/* good */
void gorm_gatt_tab_get_by_handle(gorm_gatt_tab_iter_t *iter)
{
    assert(iter);

    mutex_lock(&lock);
    iter->e = _get_entry(iter->handle);
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

gorm_gatt_entry_t *gorm_gatt_tab_find_service(uint16_t start_from)
{
    mutex_lock(&lock);
    gorm_gatt_entry_t *res = _get_table(start_from);
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

uint16_t gorm_gatt_tab_get_end_handle(gorm_gatt_entry_t *entry)
{
    if (entry->next) {
        return (entry->next->handle - 1);
    }
    if (!(entry->handle & NONSIG_FLAG) && customs) {
        return (customs->handle - 1);
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
    _print_table(&gap_entry);
    _print_table(customs);
    printf("|------------------|-------\n");
}
