/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Bluetooth Mesh Example
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "thread.h"
#include "nimble_riot.h"

#include "host/ble_hs.h"
#include "mesh/glue.h"
#include "mesh/porting.h"
#include "mesh/access.h"
#include "mesh/main.h"
#include "mesh/cfg_srv.h"
#include "mesh/health_srv.h"

#define VENDOR_CID      0x1234      /* company identifier */
#define VENDOR_PID      0x4321      /* product identifier */
#define VENDOR_VID      0x0001      /* version identifier */


static struct bt_mesh_cfg_srv _cfg_srv = {
    .beacon = BT_MESH_BEACON_ENABLED,
#if MYNEWT_VAL(BLE_MESH_RELAY)
    .relay = BT_MESH_RELAY_ENABLED,
#else
    .relay = BT_MESH_RELAY_DISABLED,
#endif
#if MYNEWT_VAL(BLE_MESH_FRIEND)
    .frnd = BT_MESH_FRIEND_ENABLED,
#else
    .frnd = BT_MESH_FRIEND_NOT_SUPPORTED,
#endif
#if MYNEWT_VAL(BLE_MESH_GATT_PROXY)
    .gatt_proxy = BT_MESH_GATT_PROXY_ENABLED,
#else
    .gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
    .default_ttl = 7,

    /* 3 transmissions with 20ms interval */
    .net_transmit = BT_MESH_TRANSMIT(2, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};

static struct bt_mesh_model_pub _health_pub;

static int _health_fault_get_cur(struct bt_mesh_model *model,
                                 uint8_t *test_id, uint16_t *company_id,
                                 uint8_t *faults, uint8_t *fault_count)
{
    (void)model;
    (void)faults;

    puts("fault_get_cur -> reporting no faults");

    *test_id = 23;
    *company_id = VENDOR_VID;
    *fault_count = 0;
    return 0;
}

static int _health_fault_get_reg(struct bt_mesh_model *model,
                                 uint16_t company_id, uint8_t *test_id,
                                 uint8_t *faults, uint8_t *fault_count)
{
    (void)model;
    (void)company_id;
    (void)faults;

    puts("fault_get_reg -> reporting no faults");

    *test_id = 24;
    *fault_count = 0;
    return 0;
}

static int _health_fault_clear(struct bt_mesh_model *model, uint16_t company_id)
{
    (void)model;
    (void)company_id;

    puts("fault_clear: doing nothing");
    return 0;
}

static int _health_fault_test(struct bt_mesh_model *model,
                              uint8_t test_id, uint16_t company_id)
{
    (void)model;

    printf("fault_test: VID 0x%04x, test ID %i\n", (int)company_id, (int)test_id);
    return 0;
}

static struct bt_mesh_health_srv_cb _health_srv = {
    .fault_get_cur = _health_fault_get_cur,
    .fault_get_reg = _health_fault_get_reg,
    .fault_clear   = _health_fault_clear,
    .fault_test    = _health_fault_test,
    .attn_on       = NULL,
    .attn_off      = NULL,
};

static struct bt_mesh_model _root_models[] = {
    BT_MESH_MODEL_CFG_SRV(&_cfg_srv),
    BT_MESH_MODEL_HEALTH_SRV(&_health_srv, &_health_pub),
};

static void _on_vendor_model_receive(struct bt_mesh_model *model,
                                     struct bt_mesh_msg_ctx *ctx,
                                     struct os_mbuf *buf)
{
    int res;
    (void)res;
    struct os_mbuf *msg = NET_BUF_SIMPLE(3);

    printf("ON-OFF model receive\n");

    bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_3(0x01, VENDOR_CID));
    os_mbuf_append(msg, buf->om_data, buf->om_len);
    res = bt_mesh_model_send(model, ctx, msg, NULL, NULL);
    assert(res == 0);

    os_mbuf_free_chain(msg);
}

static struct bt_mesh_model_pub _vendor_model_pub;

static const struct bt_mesh_model_op _vendor_model_op[] = {
    { BT_MESH_MODEL_OP_3(0x01, VENDOR_CID), 0, _on_vendor_model_receive },
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model _vendor_models[] = {
    BT_MESH_MODEL_VND(VENDOR_CID, BT_MESH_MODEL_ID_GEN_ONOFF_SRV,
                      _vendor_model_op, &_vendor_model_pub, NULL),
};

static struct bt_mesh_elem _elements[] = {
    BT_MESH_ELEM(0, _root_models, _vendor_models),
};

static const struct bt_mesh_comp _node_comp = {
    .cid = VENDOR_CID,
    .pid = VENDOR_PID,
    .vid = VENDOR_VID,
    .elem_count = ARRAY_SIZE(_elements),
    .elem = _elements,
};

static int _on_output_number(bt_mesh_output_action_t action, uint32_t number)
{
    printf("OOB action: 0x%02x\n", (int)action);
    printf("    number: %u\n", (unsigned)number);
    return 0;
}

static void _on_prov_complete(uint16_t net_idx, uint16_t addr)
{
    printf("Provisioning complete!\n");
    printf("   net_idx: %u\n", (unsigned)net_idx);
    printf("      addr: %u\n", (unsigned)addr);
}

static const uint8_t dev_uuid[16] = MYNEWT_VAL(BLE_MESH_DEV_UUID);

static const struct bt_mesh_prov _prov_cfg = {
    .uuid = dev_uuid,
    .output_size = 4,
    .output_actions = BT_MESH_DISPLAY_NUMBER,
    .output_number = _on_output_number,
    .complete = _on_prov_complete,
};


static char _stack_mesh[NIMBLE_MESH_STACKSIZE];

static void *_mesh_thread(void *arg)
{
    mesh_adv_thread(arg);
    return NULL;
}

int main(void)
{
    int res;
    (void)res;
    ble_addr_t addr;

    puts("NimBLE Mesh Example");

    /* generate and set non-resolvable private address */
    res = ble_hs_id_gen_rnd(1, &addr);
    assert(res == 0);
    res = ble_hs_id_set_rnd(addr.val);
    assert(res == 0);

    /* initialize the health publish service */
    _health_pub.msg = BT_MESH_HEALTH_FAULT_MSG(0);

    /* reload the GATT server to link our added services */
    bt_mesh_register_gatt();
    ble_gatts_start();

    /* initialize the mesh stack */
    res = bt_mesh_init(addr.type, &_prov_cfg, &_node_comp);
    if (res != 0) {
        printf("err: bt_mesh_init failed (%i)\n", res);
    }
    assert(res == 0);


    /* run mesh thread */
    thread_create(_stack_mesh, sizeof(_stack_mesh),
                  NIMBLE_MESH_PRIO,
                  THREAD_CREATE_STACKTEST,
                  _mesh_thread, NULL,
                  "nimble_mesh");

    puts("mesh init ok");

    /* advertise this node as unprovisioned */
    res = bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
    if (res != 0) {
        printf("err: bt_mesh_prov_enable failed (%i)\n", res);
    }
    assert(res == 0);

    puts("advertising this device for provisioning now...");

    return 0;
}
