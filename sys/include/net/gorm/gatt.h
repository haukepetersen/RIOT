/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_gatt
 * @ingroup     net_gorm
 * @brief       Gorm's GATT implementation
 *
 *
 * # Implementation status
 *
 * ## Descriptors
 * So far the implementation supports only descriptors of type
 * - extended properties (0x2900)
 * - presentation format (0x2904)
 * - user description (0x2901)
 *
 * @{
 *
 * @file
 * @brief       Gorm's GATT interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GORM_GATT_H
#define GORM_GATT_H

#include <stdint.h>

#include "net/ble.h"
#include "net/gorm/ll.h"
#include "net/gorm/att.h"
#include "net/gorm/uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    GATT MTU size
 *
 * TODO: make this non-static an negotiate this from the con object
 */
#define GORM_GATT_MAX_MTU               (NETDEV_BLE_PDU_MAXLEN - 4)
#define GORM_GATT_DEFAULT_MTU           (23U)

enum {
    GORM_GATT_READ,
    GORM_GATT_WRITE,
};

enum {
    GORM_GATT_OP_NOT_SUPPORTED = 0,
    GORM_GATT_OP_OK = 1,
};

typedef struct gorm_gatt_char gorm_gatt_char_t;
typedef struct gorm_gatt_desc gorm_gatt_desc_t;

typedef size_t (*gorm_gatt_char_cb_t)(const gorm_gatt_char_t *characteristic,
                                      uint8_t method,
                                      uint8_t *buf, size_t buf_len);

/* as of now, Gorm's descriptors are read-only */
typedef size_t (*gorm_gatt_desc_cb_t)(const gorm_gatt_desc_t *descriptor,
                                      uint8_t *buf, size_t buf_len);

struct gorm_gatt_desc {
    gorm_gatt_desc_cb_t cb;
    uint32_t arg;
    uint16_t type;          /**< 16-bit UUIDs only */
};

struct gorm_gatt_char {
    gorm_gatt_char_cb_t cb;
    void *arg;
    gorm_uuid_t type;
    uint8_t perm;
    uint16_t desc_cnt;
    const gorm_gatt_desc_t *desc;
};

typedef struct {
    gorm_uuid_t uuid;
    /* TODO: how to handle includes?
     *       -> maybe simply: GORM does not support includes?! */
    uint16_t char_cnt;
    const gorm_gatt_char_t *chars;
} gorm_gatt_service_t;

/**
 * @brief   Entry in the GATT table containing a service
 */
typedef struct gorm_gatt_entry {
    struct gorm_gatt_entry *next;
    const gorm_gatt_service_t *service;
    uint16_t handle;
} gorm_gatt_entry_t;



void gorm_gatt_server_init(void);

void gorm_gatt_on_data(gorm_ll_connection_t *con, gorm_buf_t *buf,
                       uint8_t *data, size_t len);






gorm_gatt_entry_t *gorm_gatt_tab_find_service(uint16_t start_from);

/**
 * @brief   Get the service identified by the service part of the given handle
 *
 * @param[in] handle    handle with valid service part
 *
 * @return  the service entry for the given handle
 * @return  NULL if service could not be found
 */
gorm_gatt_entry_t *gorm_gatt_tab_get_service(uint16_t handle);





int gorm_gatt_tab_find_char(gorm_gatt_entry_t *entry, uint16_t start_handle);

gorm_gatt_desc_t *gorm_gatt_tab_get_desc(uint16_t handle);

uint16_t gorm_gatt_tab_get_end_handle(const gorm_gatt_entry_t *entry);
uint16_t gorm_gatt_tab_get_char_handle(gorm_gatt_entry_t *entry, uint16_t num);
uint16_t gorm_gatt_tab_get_val_handle(gorm_gatt_entry_t *entry, uint16_t num);

/* TDOO: just for debugging -> remove or move ... */
void gorm_gatt_tab_print(void);




/**
 * GATT Table interface:
 *
 * - get base x: ignore lower bits and match only higher ones
 * - get exact x: use all bits and check lower ones to be 0
 */

#ifdef __cplusplus
}
#endif

#endif /* GORM_GATT_H */
/** @} */
