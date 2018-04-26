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
 * @brief       Gorm's GATT server implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#include <stdint.h>
#include <limits.h>

#include "mutex.h"
#include "net/gorm/util.h"
#include "net/gorm/l2cap.h"
#include "net/gorm/gatt.h"
#include "net/gorm/gatt/tab.h"

#define ENABLE_DEBUG                (1)
#include "debug.h"

/* pre: iter must point to service */
static size_t _write_service_attr_data(uint8_t *buf, gorm_gatt_tab_iter_t *iter)
{
    gorm_util_htoles(buf, iter->handle);
    gorm_util_htoles(&buf[2], gorm_gatt_tab_service_last_handle(iter->handle));
    return gorm_uuid_to_buf(&buf[4], &iter->s->uuid) + 4;
}

/* pre: iter must point to characteristic */
static size_t _write_char_attr_data(uint8_t *buf, gorm_gatt_tab_iter_t *iter)
{
    /* | handle | prop 1b | val handle | uuid | */
    gorm_util_htoles(&buf[0], iter->handle);
    buf[2] = iter->c->perm;
    gorm_util_htoles(&buf[3], gorm_gatt_tab_char_val_handle(iter->handle));
    return gorm_uuid_to_buf(&buf[5], &iter->c->type) + 5;
}

static void _error(gorm_ctx_t *con, gorm_buf_t *buf, uint8_t *data,
                   uint16_t handle, uint8_t code)
{
    data[1] = data[0];      /* copy request opcode */
    data[0] = BLE_ATT_ERROR_RESP;
    gorm_util_htoles(&data[2], handle);
    data[4] = code;
    gorm_l2cap_reply(con, buf, 5);
}

static void _on_mtu_req(gorm_ctx_t *con, gorm_buf_t *buf,
                        uint8_t *data, size_t len)
{
    /* this message type is used in GATT to
     * - exchange the MTU size */

    if (len != 3) {
        DEBUG("[gorm_gatt] mtu_req: invalid PDU len\n");
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        return;
    }

    DEBUG("[gorm_gatt] mtu_req: reply with MTU set to %u\n",
          GORM_GATT_DEFAULT_MTU);

    data[0] = BLE_ATT_MTU_RESP;
    gorm_util_htoles(&data[1], GORM_GATT_DEFAULT_MTU);
    gorm_l2cap_reply(con, buf, 3);
}

static void _on_read_by_group_type_req(gorm_ctx_t *con, gorm_buf_t *buf,
                                       uint8_t *data, size_t len)
{
    /* this message type is used in GATT to
     * - discover all primary services (type 0x2800)
     * Not supported so far:
     * - discovery of secondary services (0x2801) */

    if (len != 7) {
        DEBUG("[gorm_gatt] read_by_group_type_req: invalid PDU\n");
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        return;
    }

    /* parse request data */
    uint16_t start_handle = gorm_util_letohs(&data[1]);
    uint16_t end_handle = gorm_util_letohs(&data[3]);
    gorm_uuid_t uuid;
    gorm_uuid_from_buf(&uuid, &data[5], 2);
    gorm_gatt_tab_iter_t iter;
    iter.handle = gorm_gatt_tab_service_next_handle(start_handle - 1);

    DEBUG("[gorm_gatt] read_by_group_type_req: "
          "start 0x%04x end 0x%04x service 0x%04x\n",
          (int)start_handle, (int)end_handle, (int)iter.handle);

    /* validate input data */
    if ((uuid.uuid16 != BLE_DECL_PRI_SERVICE) || (iter.handle > end_handle)) {
        goto error;
    }

    /* prepare response, start by getting the first viable entry */
    gorm_gatt_tab_get(&iter);
    if ((iter.s == NULL) &&
        (iter.handle < GORM_GATT_TAB_FIRST_CUSTOM_HANDLE) &&
        (end_handle >= GORM_GATT_TAB_FIRST_CUSTOM_HANDLE)) {
        iter.handle = GORM_GATT_TAB_FIRST_CUSTOM_HANDLE;
        gorm_gatt_tab_get(&iter);
    }
    if (iter.s == NULL) {
        goto error;
    }

    /* prepare the response */
    data[0] = BLE_ATT_READ_BY_GROUP_TYPE_RESP;
    data[1] = ((uint8_t)gorm_uuid_len(&iter.s->uuid) + 4);
    size_t pos = 2;
    unsigned limit = ((GORM_GATT_DEFAULT_MTU - pos) / data[1]);

    /* fit as many services with the same length UUID into the response */
    while (iter.s && (limit > 0)) {
        DEBUG("[gorm_gatt]                       : "
              "found 0x%04x\n", (int)iter.s->handle);
        pos += _write_service_attr_data(&data[pos], &iter);
        --limit;
        gorm_gatt_tab_get_next(&iter, end_handle);
    }

    gorm_l2cap_reply(con, buf, pos);
    return;

error:
    DEBUG("[gorm_gatt]                       : no entry found\n");
    _error(con, buf, data, start_handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
}

static void _on_read_by_type_req(gorm_ctx_t *con, gorm_buf_t *buf,
                                 uint8_t *data, size_t len)
{
    /* this message type is used in GATT to
     * - discover all characteristics of a service
     * Not supported so far:
     * - read char using characteristic UUID (?)
     * - discover characteristic by UUID
     * - find included services (not supported in Gorm for now) */

    if ((len != 7) && (len != 21)) {
        DEBUG("[gorm_gatt] read_by_type_req: invalid request length\n");
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        return;
    }

    /* parse the request data */
    uint16_t start_handle = gorm_util_letohs(&data[1]);
    uint16_t end_handle = gorm_util_letohs(&data[3]);
    gorm_uuid_t uuid;
    gorm_uuid_from_buf(&uuid, &data[5], (len - 5));

    DEBUG("[gorm_gatt] read_by_type_req: type ");
#if ENABLE_DEBUG
    gorm_uuid_print(&uuid);
#endif
    DEBUG("\n");

    if (gorm_uuid_eq16(&uuid, BLE_DECL_CHAR)) {
        gorm_gatt_tab_iter_t iter;
        iter.handle = gorm_gatt_tab_char_closest_handle(start_handle);

        DEBUG("[gorm_gatt]                 : CHAR_DECL - "
              "start 0x%04x end 0x%04x char 0x%04x\n",
              (int)start_handle, (int)end_handle, (int)iter.handle);

        if (iter.handle > end_handle) {
            goto error;
        }

        gorm_gatt_tab_get(&iter);

        if (iter.c == NULL) {
            goto error;
        }

        size_t uuidlen = gorm_uuid_len(&iter.c->type);
        data[0] = BLE_ATT_READ_BY_TYPE_RESP;
        data[1] = (uuidlen + 5);
        size_t pos = 2;
        unsigned limit = ((GORM_GATT_DEFAULT_MTU - 2) / data[1]);

        while (iter.c && (limit > 0) &&
               (gorm_uuid_len(&iter.c->type) == uuidlen)) {
            DEBUG("[gorm_gatt]                 : "
                  "found 0x%04x\n", (int)iter.handle);
            pos += _write_char_attr_data(&data[pos], &iter);
            --limit;
            gorm_gatt_tab_get_next(&iter, end_handle);
        }

        gorm_l2cap_reply(con, buf, pos);
        return;
    }

error:
    DEBUG("[gorm_gatt]                 : no entry found\n");
    _error(con, buf, data, start_handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
}

static void _on_find_info_req(gorm_ctx_t *con, gorm_buf_t *buf,
                              uint8_t *data, size_t len)
{
    /* this message type is used in GATT to
     * - discover all characteristic descriptors */

    if (len != 5) {
        DEBUG("[gorm_gatt] find_info_req: invalid PDU\n");
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        return;
    }

    /* get input parameters */
    uint16_t start_handle = gorm_util_letohs(&data[1]);
    uint16_t end_handle = gorm_util_letohs(&data[3]);
    gorm_gatt_tab_iter_t iter;
    iter.handle = gorm_gatt_tab_desc_closest_handle(start_handle);

    DEBUG("[gorm_gatt] find_info_req: "
          "start 0x%04x end 0x%04x desc 0x%04x\n",
          (int)start_handle, (int)end_handle, (int)iter.handle);

    /* parse first fitting descriptor handle and validate it */
    if (iter.handle > end_handle) {
        goto error;
    }

    /* read descriptor from table */
    gorm_gatt_tab_get(&iter);
    if (iter.d == NULL) {
        goto error;
    }

    /* prepare response */
    data[0] = BLE_ATT_FIND_INFO_RESP;
    data[1] = BLE_ATT_FORMAT_U16;
    size_t pos = 2;
    size_t limit = ((GORM_GATT_DEFAULT_MTU - 2) / 4);

    while (iter.d && (limit > 0)) {
        DEBUG("[gorm_gatt]              : found 0x%04x\n", (int)iter.handle);
        gorm_util_htoles(&data[pos], iter.handle);
        gorm_util_htoles(&data[pos + 2], iter.d->type);
        pos += 4;
        limit--;
        gorm_gatt_tab_get_next(&iter, end_handle);
    }

    gorm_l2cap_reply(con, buf, pos);
    return;

error:
    DEBUG("[gorm_gatt]              : no entry found\n");
    _error(con, buf, data, start_handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
}

void _on_find_by_type_val(gorm_ctx_t *con, gorm_buf_t *buf,
                          uint8_t *data, size_t len)
{
    /* this message type is used in GATT to
     * - discover primary services by service UUID */

    if ((len != 9) && (len != 23)) {
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        DEBUG("[gorm_gatt] find_by_type_val: invalid PDU\n");
        return;
    }

    /* parse PDU */
    uint16_t start_handle = gorm_util_letohs(&data[1]);
    uint16_t service_handle = gorm_gatt_tab_service_next_handle(start_handle - 1);
    uint16_t end_handle = gorm_util_letohs(&data[3]);
    uint16_t type = gorm_util_letohs(&data[5]);
    gorm_uuid_t uuid;
    gorm_uuid_from_buf(&uuid, &data[7], (len - 7));

#if ENABLE_DEBUG
    DEBUG("[gorm_gatt] find_by_type_val: type 0x%04x filter ", (int)type);
    gorm_uuid_print(&uuid);
#endif
    DEBUG("\n[gorm_gatt]                 : "
          "start 0x%02x end 0x%02x service_handle 0x%04x\n",
          (int)start_handle, (int)end_handle, (int)service_handle);

    /* validate input data */
    if ((type != BLE_DECL_PRI_SERVICE) || (service_handle > end_handle)) {
        goto error;
    }

    /* find first matching service */
    gorm_gatt_service_t *s = gorm_gatt_tab_service_by_uuid(&uuid,
                                                           service_handle,
                                                           end_handle);
    if (s == NULL) {
        DEBUG("[gorm_gatt] _on_find_by_type_val: no service with UUID found\n");
        goto error;
    }

    /* prepare results */
    DEBUG("[gorm_gatt]                 : found 0x%04x\n", (int)service_handle);
    data[0] = BLE_ATT_FIND_BY_VAL_RESP;
    gorm_util_htoles(&data[1], service_handle);
    gorm_util_htoles(&data[3], gorm_gatt_tab_service_last_handle(service_handle));
    gorm_l2cap_reply(con, buf, 5);
    return;

error:
    DEBUG("[gorm_gatt]                 : no service with given UUID found\n");
    _error(con, buf, data, start_handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
}

static void _on_read_req(gorm_ctx_t *con, gorm_buf_t *buf,
                         uint8_t *data, size_t len)
{
    /* this message type is used in GATT for:
     * - read characteristic descriptor
     * - read characteristic value */
    if (len != 3) {
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        return;
    }

    /* parse handle from request */
    gorm_gatt_tab_iter_t iter;
    iter.handle = gorm_util_letohs(&data[1]);

    /* read data */
    gorm_gatt_tab_get(&iter);
    data[0] = BLE_ATT_READ_RESP;

    /* if handle belongs to a characteristic value, check permissions and read
     * that value when allowed */
    if (gorm_gatt_tab_is_char_val(&iter)) {
        DEBUG("[gorm_gatt] read_req: reading handle 0x%04x (char val)\n",
              (int)iter.handle);
        /* TODO: check read permissions and authen/author */
        /* check if characteristic value is readable */
        if (iter.c->perm & BLE_ATT_READ) {
            DEBUG("[gorm_gatt]         : OK to read\n");
            size_t len = iter.c->cb(iter.c, GORM_GATT_READ,
                                    &data[1], (GORM_GATT_DEFAULT_MTU - 1));
            gorm_l2cap_reply(con, buf, (len + 1));
        }
        else {
            DEBUG("[gorm_gatt]         : READ operation not permitted\n");
            _error(con, buf, data, iter.handle, BLE_ATT_READ_NOT_PERMITTED);
        }
    }
    /* if the handle belongs to a descriptor, we read it */
    else if (gorm_gatt_tab_is_desc(&iter)) {
        DEBUG("[gorm_gatt] read_req: reading handle 0x%04x (description)\n",
              (int)iter.handle);
        /* note: we assume all descriptors are readable without permissions */
        size_t len = iter.d->cb(iter.d, &data[1], (GORM_GATT_DEFAULT_MTU - 1));
        gorm_l2cap_reply(con, buf, (len + 1));
    }
    else {
        DEBUG("[gorm_gatt] read_req: unable to find handle 0x%04x\n",
              (int)iter.handle);
        _error(con, buf, data, iter.handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
    }
}

static void _on_write_req(gorm_ctx_t *con, gorm_buf_t *buf,
                          uint8_t *data, size_t len)
{
    if (len < 3) {
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        return;
    }

    /* parse input data */
    gorm_gatt_tab_iter_t iter;
    iter.handle = gorm_util_letohs(&data[1]);

    /* write data if applicable */
    gorm_gatt_tab_get(&iter);

    if (gorm_gatt_tab_is_char_val(&iter)) {
        DEBUG("[gorm_gatt] write_req: writing to handle 0x%04x (char value)\n",
              (int)iter.handle);
        /* make sure the value is writable and we are allowed to do so */
        /* TODO: check encryption, authentication, and authorization... */
        if (iter.c->perm & BLE_ATT_WRITE) {
            DEBUG("[gorm_gatt]          : OK to write\n");
            iter.c->cb(iter.c, GORM_GATT_WRITE, &data[3], (len - 3));
            data[0] = BLE_ATT_WRITE_RESP;
            gorm_l2cap_reply(con, buf, 1);
        }
        else {
            DEBUG("[gorm_gatt]          : WRITE operation not permitted\n");
            _error(con, buf, data, iter.handle, BLE_ATT_WRITE_NOT_PERMITTED);
        }
    }
    else {
        DEBUG("[gorm_gatt] write_req: unable to find handle 0x%04x\n",
              (int)iter.handle);
        _error(con, buf, data, iter.handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
    }
}

void gorm_gatt_server_init(void)
{
    gorm_gatt_services_init();
    DEBUG("[gorm_gatt] initialization successful\n");
}

void gorm_gatt_on_data(gorm_ctx_t *con, gorm_buf_t *buf,
                       uint8_t *data, size_t len)
{
    switch (data[0]) {
        case BLE_ATT_MTU_REQ:
            _on_mtu_req(con, buf, data, len);
            break;
        case BLE_ATT_FIND_INFO_REQ:
            _on_find_info_req(con, buf, data, len);
            break;
        case BLE_ATT_FIND_BY_VAL_REQ:
            _on_find_by_type_val(con, buf, data, len);
            break;
        case BLE_ATT_READ_BY_TYPE_REQ:
            _on_read_by_type_req(con, buf, data, len);
            break;
        case BLE_ATT_READ_REQ:
            _on_read_req(con, buf, data, len);
            break;
        case BLE_ATT_READ_BY_GROUP_TYPE_REQ:
            _on_read_by_group_type_req(con, buf, data, len);
            break;
        case BLE_ATT_WRITE_REQ:
            _on_write_req(con, buf, data, len);
            break;
        case BLE_ATT_READ_BLOB_REQ:
        case BLE_ATT_READ_MUL_REQ:
        case BLE_ATT_PREP_WRITE_REQ:
        case BLE_ATT_WRITE_COMMAND:
        case BLE_ATT_EXEC_WRITE_REQ:
        case BLE_ATT_VAL_NOTIFICATION:
        case BLE_ATT_VAL_INDICATION:
        case BLE_ATT_VAL_CONFIRMATION:
        case BLE_ATT_SIGNED_WRITE_CMD:
            DEBUG("[gorm_gatt] on_data: unknown opcode %i\n", (int)data[0]);
            _error(con, buf, data, 0, BLE_ATT_REQUEST_NOT_SUP);
            gorm_buf_return(buf);
            break;

        case BLE_ATT_ERROR_RESP:
        case BLE_ATT_MTU_RESP:
        case BLE_ATT_FIND_INFO_RESP:
        case BLE_ATT_FIND_BY_VAL_RESP:
        case BLE_ATT_READ_BY_TYPE_RESP:
        case BLE_ATT_READ_RESP:
        case BLE_ATT_READ_BLOB_RESP:
        case BLE_ATT_READ_MUL_RESP:
        case BLE_ATT_READ_BY_GROUP_TYPE_RESP:
        case BLE_ATT_WRITE_RESP:
        case BLE_ATT_PREP_WRITE_RESP:
        case BLE_ATT_EXEC_WRITE_RESP:
        default:
            /* we silently drop any response we get (as we are the server...) */
            DEBUG("[gorm_gatt] on_data: got undefined response, ignoring it\n");
            gorm_buf_return(buf);
            break;
    }
}
