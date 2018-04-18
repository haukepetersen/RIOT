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

/* REOMVE! */
#include "xtimer.h"

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
    if (len != 3) {
        DEBUG("[gatt_server] _on_mtu_req: invalid PDU len\n");
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        return;
    }

    data[0] = BLE_ATT_MTU_RESP;
    gorm_util_htoles(&data[1], GORM_GATT_DEFAULT_MTU);
    DEBUG("[gatt_server] _on_mtu_req: sending reply now\n");
    gorm_l2cap_reply(con, buf, 3);
}

/* discover all primary services on the server */
static void _on_read_by_group_type_req(gorm_ctx_t *con, gorm_buf_t *buf,
                                       uint8_t *data, size_t len)
{
    /* we only allow discovery of primary (0x2800) and secondary (0x2801)
     * service through this method, hence we use only 16-bit UUIDs here */
    if (len != 7) {
        DEBUG("[gatt_server] _on_read_by_group_type_req: invalid PDU len\n");
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        return;
    }

    /* parse request data */
    uint16_t start_handle = gorm_util_letohs(&data[1]);
    uint16_t first_handle = gorm_gatt_tab_service_next_handle(start_handle - 1);
    uint16_t end_handle = gorm_util_letohs(&data[3]);
    gorm_uuid_t uuid;
    gorm_uuid_from_buf(&uuid, &data[5], 2);

    /* validate input data */
    if ((uuid.uuid16 != BLE_DECL_PRI_SERVICE) || (first_handle > end_handle)) {
        DEBUG("[gatt_server] _on_read_by_group_type_req: invalid input\n");
        goto error;
    }

    /* prepare response, start by getting the first viable entry */
    gorm_gatt_tab_iter_t iter;
    gorm_gatt_tab_get(&iter, first_handle);
    if (iter.s == NULL) {
        DEBUG("[gatt_server] _on_read_by_group_type_req: no entry found\n");
        goto error;
    }

    /* prepare the response */
    data[0] = BLE_ATT_READ_BY_GROUP_TYPE_RESP;
    data[1] = ((uint8_t)gorm_uuid_len(&iter.s->uuid) + 4);
    size_t pos = 2;
    unsigned limit = ((GORM_GATT_DEFAULT_MTU - pos) / data[1]);

    /* fit as many services with the same length UUID into the response */
    while (iter.s && (limit > 0)) {
        pos += _write_service_attr_data(&data[pos], &iter);
        --limit;
        gorm_gatt_tab_get_next(&iter, end_handle);
    }

    gorm_l2cap_reply(con, buf, pos);
    return;

error:
    _error(con, buf, data, start_handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
}

static void _on_read_by_type_req(gorm_ctx_t *con, gorm_buf_t *buf,
                                 uint8_t *data, size_t len)
{
    /* this message type is used in GATT to
     * - find included services (not supported in Gorm for now)
     * - discover all characteristics of a service
     * - discover characteristic by UUID
     * - read char using characteristic UUID (?) */

    /* expected size is 7 or 21 byte (16- or 128-bit UUID) */
    if ((len != 7) && (len != 21)) {
        DEBUG("[gatt_server] read_by_type_req: invalid request length\n");
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        return;
    }

    /* parse the request data */
    uint16_t start_handle = gorm_util_letohs(&data[1]);
    uint16_t end_handle = gorm_util_letohs(&data[3]);
    gorm_uuid_t uuid;
    gorm_uuid_from_buf(&uuid, &data[5], (len - 5));

    if (gorm_uuid_eq16(&uuid, BLE_DECL_CHAR)) {
        uint16_t char_handle = gorm_gatt_tab_char_next_handle(start_handle - 1);
        if (char_handle > end_handle) {
            goto error;
        }

        gorm_gatt_tab_iter_t iter;
        gorm_gatt_tab_get(&iter, char_handle);

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
            pos += _write_char_attr_data(&data[pos], &iter);
            --limit;
            gorm_gatt_tab_get_next(&iter, end_handle);
        }

        gorm_l2cap_reply(con, buf, pos);
        return;
    }

error:
    /* if nothing fitting was found, we return the default error here */
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
    uint16_t handle = gorm_util_letohs(&data[1]);

    /* read data */
    gorm_gatt_tab_iter_t iter;
    gorm_gatt_tab_get(&iter, handle);
    data[0] = BLE_ATT_READ_RESP;

    /* if handle belongs to a characteristic value, check permissions and read
     * that value when allowed */
    if (gorm_gatt_tab_is_char_val(&iter)) {
        /* TODO: check read permissions and authen/author */
        /* check if characteristic value is readable */
        if (iter.c->perm & BLE_ATT_READ) {
            size_t len = iter.c->cb(iter.c, GORM_GATT_READ,
                                    &data[1], (GORM_GATT_DEFAULT_MTU - 1));
            gorm_l2cap_reply(con, buf, (len + 1));
        }
        else {
            _error(con, buf, data, handle, BLE_ATT_READ_NOT_PERMITTED);
        }
    }
    /* if the handle belongs to a descriptor, we read it */
    else if (gorm_gatt_tab_is_decl(&iter)) {
        /* note: we assume all descriptors are readable without permissions */
        size_t len = iter.d->cb(iter.d, &data[1], (GORM_GATT_DEFAULT_MTU - 1));
        gorm_l2cap_reply(con, buf, (len + 1));
    }
    else {
        _error(con, buf, data, handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
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
    uint16_t handle = gorm_util_letohs(&data[1]);

    /* write data if applicable */
    gorm_gatt_tab_iter_t iter;
    gorm_gatt_tab_get(&iter, handle);

    if (gorm_gatt_tab_is_char_val(&iter)) {
        /* make sure the value is writable and we are allowed to do so */
        /* TODO: check encryption, authentication, and authorization... */
        if (iter.c->perm & BLE_ATT_WRITE) {
            iter.c->cb(iter.c, GORM_GATT_WRITE, &data[3], (len - 3));
            data[0] = BLE_ATT_WRITE_RESP;
            gorm_l2cap_reply(con, buf, 1);
        }
        else {
            _error(con, buf, data, handle, BLE_ATT_WRITE_NOT_PERMITTED);
        }
    }
    else {
        _error(con, buf, data, handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
    }
}

static void _on_find_info_req(gorm_ctx_t *con, gorm_buf_t *buf,
                              uint8_t *data, size_t len)
{

    if (len != 5) {
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        return;
    }

    /* get input parameters */
    uint16_t start_handle = gorm_util_letohs(&data[1]);
    uint16_t end_handle = gorm_util_letohs(&data[3]);

    /* parse first fitting descriptor handle and validate it */
    uint16_t desc_handle = gorm_gatt_tab_desc_next_handle(start_handle - 1);
    if (desc_handle > end_handle) {
        goto error;
    }

    /* read descriptor from table */
    gorm_gatt_tab_iter_t iter;
    gorm_gatt_tab_get(&iter, start_handle);
    if (iter.d == NULL) {
        goto error;
    }

    /* prepare response */
    data[0] = BLE_ATT_FIND_INFO_RESP;
    data[1] = BLE_ATT_FORMAT_U16;
    size_t pos = 2;
    size_t limit = ((GORM_GATT_DEFAULT_MTU - 2) / 4);

    while (iter.d && (limit > 0)) {
        gorm_util_htoles(&data[pos], iter.handle);
        gorm_util_htoles(&data[pos + 2], iter.d->type);
        pos += 4;
        limit--;
        gorm_gatt_tab_get_next(&iter, end_handle);
    }

    gorm_l2cap_reply(con, buf, pos);
    return;

error:
    _error(con, buf, data, start_handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
}

void _on_find_by_type_val(gorm_ctx_t *con, gorm_buf_t *buf,
                          uint8_t *data, size_t len)
{
    if ((len != 9) && (len != 23)) {
        _error(con, buf, data, 0, BLE_ATT_INVALID_PDU);
        DEBUG("[gorm_gatt] _on_find_by_type_val: invalid PDU\n");
        return;
    }

    /* parse PDU */
    uint16_t start_handle = gorm_util_letohs(&data[1]);
    uint16_t service_handle = gorm_gatt_tab_service_next_handle(start_handle - 1);
    uint16_t end_handle = gorm_util_letohs(&data[3]);
    uint16_t type = gorm_util_letohs(&data[5]);
    gorm_uuid_t uuid;
    gorm_uuid_from_buf(&uuid, &data[7], (len - 7));

    DEBUG("[gorm_gatt] _on_find_by_type_val: start 0x%02x, end 0x%02x\n",
          (int)handle, (int)end_handle);

    /* validate input data */
    if ((type != BLE_DECL_PRI_SERVICE) || (service_handle > end_handle)) {
        goto error;
    }

    /* find first matching service */
    gorm_gatt_service_t *s = gorm_gatt_tab_service_by_uuid(&iter, &uuid,
                                                           start_handle,
                                                           end_handle);

    if (iter.e == NULL) {
        DEBUG("[gorm_gatt] _on_find_by_type_val: no service with UUID found\n");
        goto error;
    }

    /* prepare results */
    data[0] = BLE_ATT_FIND_BY_VAL_RESP;
    gorm_util_htoles(&data[1], iter.s->handle);
    gorm_util_htoles(&data[3], gorm_gatt_tab_service_last_handle(s->handle));
    gorm_l2cap_reply(con, buf, 5);
    DEBUG("[grom_gatt] _on_find_by_type_val: found service 0x%02x\n", (int)iter.handle);
    return;

error:
    _error(con, buf, data, handle, BLE_ATT_ATTRIBUTE_NOT_FOUND);
}

void gorm_gatt_server_init(void)
{
    gorm_gatt_services_init();
    DEBUG("[gorm_gatt] initialization successful\n");
}

void gorm_gatt_on_data(gorm_ctx_t *con, gorm_buf_t *buf,
                       uint8_t *data, size_t len)
{
    uint32_t now = xtimer_now_usec();

    switch (data[0]) {
        /* TODO: restore order ... */
        case BLE_ATT_MTU_REQ:
            DEBUG("[gatt_server] _on_mtu_req()\n");
            _on_mtu_req(con, buf, data, len);
            break;
        case BLE_ATT_READ_BY_GROUP_TYPE_REQ:
            DEBUG("[gatt_server] _on_read_by_group_type_req()\n");
            _on_read_by_group_type_req(con, buf, data, len);
            break;
        case BLE_ATT_READ_BY_TYPE_REQ:
            DEBUG("[gatt_server] _on_read_by_type_req()\n");
            _on_read_by_type_req(con, buf, data, len);
            break;
        case BLE_ATT_READ_REQ:
            DEBUG("[gatt_server] _on_read_req()\n");
            _on_read_req(con, buf, data, len);
            break;
        case BLE_ATT_FIND_INFO_REQ:
            DEBUG("[gatt_server] _on_find_info_req()\n");
            _on_find_info_req(con, buf, data, len);
            break;
        case BLE_ATT_WRITE_REQ:
            DEBUG("[gatt_server] _on_write_req()\n");
            _on_write_req(con, buf, data, len);
            break;
        case BLE_ATT_FIND_BY_VAL_REQ:
            DEBUG("[gatt_server] _on_find_by_type_val()\n");
            _on_find_by_type_val(con, buf, data, len);
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

    uint32_t diff = xtimer_now_usec() - now;
    DEBUG("[gatt_server] on_data() done (took %u us)\n", (unsigned)diff);
}
