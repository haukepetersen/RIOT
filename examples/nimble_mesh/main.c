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

#include "host/ble_hs.h"
#include "mesh/glue.h"
#include "mesh/access.h"
#include "mesh/main.h"

#define VENDOR_CID      0x1234      /* company identifier */
#define VENDOR_PID      0x4321      /* product identifier */
#define VENDOR_VID      0x0001      /* version identifier */

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


// static struct bt_mesh_model _models[] = {

// };

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
    BT_MESH_ELEM(0, NULL, _vendor_models),
};

static const struct bt_mesh_comp _node_comp = {
    .cid = VENDOR_CID,
    .pid = VENDOR_PID,
    .vid = VENDOR_VID,
    .elem_count = ARRAY_SIZE(_elements),
    .elem = _elements,
};

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

    /* initialize the mesh stack */
    res = bt_mesh_init(addr.type, &_prov_cfg, &_node_comp);
    if (res != 0) {
        printf("err: bt_mesh_init failed (%i)\n", res);
    }
    assert(res == 0);

    /* advertise this node as unprovisioned */
    res = bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
    if (res != 0) {
        printf("err: bt_mesh_prov_enable failed (%i)\n", res);
    }
    assert(res == 0);

    puts("advertising this device for provisioning now...");

    return 0;
}
