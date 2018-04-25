/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gorm_gatt GATT for Gorm
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

#ifndef NET_GORM_GATT_H
#define NET_GORM_GATT_H

#include <stdint.h>

#include "net/ble.h"
#include "net/gorm.h"
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
 * @{
 */
#define GORM_GATT_MAX_MTU               (NETDEV_BLE_PDU_MAXLEN - 4)
#define GORM_GATT_DEFAULT_MTU           (23U)
/** @} */

/**
 * @brief   Access method flags used by Gorm's GATT implementation
 */
enum {
    GORM_GATT_READ,         /**< read operation */
    GORM_GATT_WRITE,        /**< write operation */
};

/**
 * @brief   Type definition for GATT characteristic instances
 */
typedef struct gorm_gatt_char gorm_gatt_char_t;

/**
 * @brief   Type declaration for GATT descriptor instances
 */
typedef struct gorm_gatt_desc gorm_gatt_desc_t;

/**
 * @brief   Type declaration for GATT service instances
 */
typedef struct gorm_gatt_service gorm_gatt_service_t;

/**
 * @brief   Callback signature for access to GATT characteristics
 *
 * @param[in] characteristic    characteristic that is being accessed
 * @param[in] method            operation to carry out
 * @param[in,out] buf           buffer to read/write data to/from
 * @param[in] buf_len           (max) size of @buf
 *
 * @return  number of bytes read from/written to @p buf
 * @return  0 if method is not supported
 */
typedef size_t (*gorm_gatt_char_cb_t)(const gorm_gatt_char_t *characteristic,
                                      uint8_t method,
                                      uint8_t *buf, size_t buf_len);

/**
 * @brief   Callback for reading characteristic descriptors
 *
 * As of now, Gorm supports only read-only descriptors, so this callback is
 * only used for READ operations.
 *
 * @param[in] descriptor        descriptor that is read
 * @param[in] buf               buffer to write result to
 * @param[in] buf_len           size of @p buf
 *
 * @return  number of bytes written to @p buf
 */
typedef size_t (*gorm_gatt_desc_cb_t)(const gorm_gatt_desc_t *descriptor,
                                      uint8_t *buf, size_t buf_len);

/**
 * @brief   Structure for defining instances of GATT characteristic descriptors
 */
struct gorm_gatt_desc {
    gorm_gatt_desc_cb_t cb;     /**< function called on READ operations */
    uint32_t arg;               /**< user argument passed to the callback */
    uint16_t type;              /**< descriptor type, 16-bit UUIDs only */
};

/**
 * @brief   Structure for defining instances of GATT characteristics
 */
struct gorm_gatt_char {
    gorm_gatt_char_cb_t cb;     /**< function called on characteristic access */
    void *arg;                  /**< user argument passed to the callback */
    gorm_uuid_t type;           /**< access method (READ or WRITE) */
    uint8_t perm;               /**< permission flags */
    uint16_t desc_cnt;          /**< number of descriptors for this char */
    const gorm_gatt_desc_t *desc;   /**< array holding descriptors */
};

/**
 * @brief   Structure for defining instances of GATT services
 */
struct gorm_gatt_service {
    struct gorm_gatt_service *next; /**< next service in list */
    gorm_uuid_t uuid;               /**< UUID of the service */
    uint16_t handle;                /**< internal base handle of the service */
    uint16_t char_cnt;              /**< number of contained characteristics */
    const gorm_gatt_char_t *chars;  /**< pointer to characteristics */
};

/**
 * @brief   Initialize Gorm's GATT server
 */
void gorm_gatt_server_init(void);

/**
 * @brief   Initialize the mandatory, included GATT services (GAP and ATT)
 */
void gorm_gatt_services_init(void);

/**
 * @brief   Handle incoming GATT data packets
 *
 * @param[in] con       open connection context
 * @param[in,out] buf   packet buffer holding the incoming packet
 * @param[in,out] data  actual GATT payload, skipping lower layer parts of PDU
 * @param[in] len       length of @p data in bytes
 */
void gorm_gatt_on_data(gorm_ctx_t *con, gorm_buf_t *buf,
                       uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* NET_GORM_GATT_H */
/** @} */
