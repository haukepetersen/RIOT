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
#include "periph/gpio.h"
#include "event/callback.h"

#include "host/ble_hs.h"
#include "mesh/glue.h"
#include "mesh/porting.h"
#include "mesh/access.h"
#include "mesh/main.h"
#include "mesh/cfg_srv.h"
#include "mesh/health_srv.h"

// #define VENDOR_CID      0x1234      /* company identifier */
#define VENDOR_CID      0x05C3
// #define VENDOR_PID      0x4321      /* product identifier */
// #define VENDOR_VID      0x0001      /* version identifier */

#define DEBOUNCE_DELAY          (100 * US_PER_MS)   /* 100ms */


/* button and pin maps */
static const gpio_t _btn[] = { BTN0_PIN, BTN1_PIN, BTN2_PIN, BTN3_PIN };
static const gpio_t _led[] = { LED0_PIN, LED1_PIN, LED2_PIN, LED3_PIN };
static xtimer_t _debounce[4];
static event_queue_t _btn_evevtq;
static event_callback_t _btn_event[4];


static uint8_t _trans_id = 0;
static int _is_provisioned = 0;


static void _on_debounce(void *arg)
{
    unsigned num = (unsigned)arg;
    gpio_irq_enable(_btn[num]);
}

static uint8_t _led_read(unsigned pin)
{
    return (gpio_read(_led[pin])) ? 0 : 1;      /* active low... */
}

static void _led_write(unsigned pin, uint8_t state)
{
    gpio_write(_led[pin], !state);
}

static struct bt_mesh_cfg_srv _cfg_srv = {
    .relay = BT_MESH_RELAY_ENABLED,
    .beacon = BT_MESH_BEACON_ENABLED,
// #if MYNEWT_VAL(BLE_MESH_RELAY)
// #else
    // .relay = BT_MESH_RELAY_DISABLED,
// #endif
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
    *company_id = VENDOR_CID;
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

static void _healt_attn_on(struct bt_mesh_model *model)
{
    (void)model;
    puts("attention ON");
}

static void _health_attn_off(struct bt_mesh_model *model)
{
    (void)model;
    puts("attention OFF");
}

static const struct bt_mesh_health_srv_cb _health_srv_cb = {
    .fault_get_cur = _health_fault_get_cur,
    .fault_get_reg = _health_fault_get_reg,
    .fault_clear   = _health_fault_clear,
    .fault_test    = _health_fault_test,
    .attn_on       = _healt_attn_on,
    .attn_off      = _health_attn_off,
};

static struct bt_mesh_health_srv _health_srv = {
    .cb = &_health_srv_cb,
};

static struct bt_mesh_model _models_root[] = {
    BT_MESH_MODEL_CFG_SRV(&_cfg_srv),
    BT_MESH_MODEL_HEALTH_SRV(&_health_srv, &_health_pub),
};

static void _led_op_get(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct os_mbuf *buf)
{
    (void)buf;
    unsigned pin = (unsigned)model->user_data;
    struct os_mbuf *msg = NET_BUF_SIMPLE(2 + 1 + 4);
    uint8_t state = _led_read(pin);

    printf("OP: LED get (pin %u)\n", pin);

    bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_2(0x82, 0x01));
    net_buf_simple_add_u8(msg, state);
    int res = bt_mesh_model_send(model, ctx, msg, NULL, NULL);
    if (res != 0) {
        printf("Error: _led_op_get(): bt_mesh_model_send() failed (%i)\n", res);
        assert(0);
    }
    os_mbuf_free_chain(msg);
}

static void _led_op_set_unack(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct os_mbuf *buf)
{
    (void)ctx;
    unsigned pin = (unsigned)model->user_data;
    uint8_t state = _led_read(pin);
    uint8_t new_state = net_buf_simple_pull_u8(buf);

    printf("OP: LED set (pin %u)\n", pin);
    _led_write(pin, new_state);

    if ((state != new_state) && (model->pub->addr != BT_MESH_ADDR_UNASSIGNED)) {
        struct os_mbuf *msg = model->pub->msg;
        bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_2(0x82, 0x04));
        net_buf_simple_add_u8(msg, new_state);
        int res = bt_mesh_model_publish(model);
        assert(res == 0);
    }
}

static void _led_op_set(struct bt_mesh_model *model,
                              struct bt_mesh_msg_ctx *ctx,
                              struct os_mbuf *buf)
{
    printf("OP: LED set unack (pin %u)\n", (unsigned)model->user_data);
    _led_op_set_unack(model, ctx, buf);
    _led_op_get(model, ctx, buf);
}

static void _btn_status(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct os_mbuf *buf)
{
    (void)ctx;
    (void)buf;
    unsigned pin = (unsigned)model->user_data;

    printf("OP: BTN status (pin %u)\n", pin);
}


static const struct bt_mesh_model_op _btn_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x04), 1, _btn_status },
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op _led_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x01), 0, _led_op_get },
    { BT_MESH_MODEL_OP_2(0x82, 0x02), 2, _led_op_set },
    { BT_MESH_MODEL_OP_2(0x82, 0x03), 2, _led_op_set_unack },
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model_pub _s_pub[8];

static struct bt_mesh_model _models_s0[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, _btn_op,
                  &_s_pub[0], (void *)0),
};

static struct bt_mesh_model _models_s1[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, _btn_op,
                  &_s_pub[1], (void *)1),
};

static struct bt_mesh_model _models_s2[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, _btn_op,
                  &_s_pub[2], (void *)2),
};

static struct bt_mesh_model _models_s3[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, _btn_op,
                  &_s_pub[3], (void *)3),
};

static struct bt_mesh_model _models_s4[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, _led_op,
                  &_s_pub[4], (void *)0),
};

static struct bt_mesh_model _models_s5[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, _led_op,
                  &_s_pub[5], (void *)1),
};

static struct bt_mesh_model _models_s6[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, _led_op,
                  &_s_pub[6], (void *)2),
};

static struct bt_mesh_model _models_s7[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, _led_op,
                  &_s_pub[7], (void *)3),
};

static struct bt_mesh_model *_btn_models[] = {
    &_models_s0[0],
    &_models_s1[0],
    &_models_s2[0],
    &_models_s3[0],
};

static struct bt_mesh_elem _elements[] = {
    BT_MESH_ELEM(0, _models_root, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_s0, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_s1, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_s2, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_s3, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_s4, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_s5, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_s6, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_s7, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp _node_comp = {
    .cid = VENDOR_CID,
    // .pid = VENDOR_PID,
    // .vid = VENDOR_VID,
    .elem = _elements,
    .elem_count = ARRAY_SIZE(_elements),
};

static int _on_output_number(bt_mesh_output_action_t action, uint32_t number)
{
    printf("OOB action: 0x%02x\n", (int)action);
    printf("    number: %u\n", (unsigned)number);
    return 0;
}

static int _on_output_string(const char *str)
{
    printf("OOB action: string\n");
    printf("    string: %s\n", str);
    return 0;
}

static int _on_input_required(bt_mesh_input_action_t act, u8_t size)
{
    printf("Provisioning: input required, act %i, size %i\n",
           (int)act, (int)size);
    return 0;
}

static void _on_link_open(bt_mesh_prov_bearer_t bearer)
{
    printf("Provisioning: link open, bearer: 0x%02x\n", (int)bearer);
}

static void _on_link_close(bt_mesh_prov_bearer_t bearer)
{
    printf("Provisioning: link close, bearer: 0x%02x\n", (int)bearer);
}

static void _on_prov_complete(uint16_t net_idx, uint16_t addr)
{
    printf("Provisioning complete!\n");
    printf("   net_idx: %u\n", (unsigned)net_idx);
    printf("      addr: %u\n", (unsigned)addr);
    _is_provisioned = 1;
}

static void _on_reset(void)
{
    puts("Provisioning: reset");
}

static const uint8_t dev_uuid[16] = MYNEWT_VAL(BLE_MESH_DEV_UUID);

static const struct bt_mesh_prov _prov_cfg = {
    .uuid           = dev_uuid,
    .output_size    = 4,
    .output_actions = BT_MESH_DISPLAY_NUMBER,
    // .output_size = 0,
    // .output_actions = 0,
    .output_number  = _on_output_number,
    .output_string  = _on_output_string,
    .input          = _on_input_required,
    .link_open      = _on_link_open,
    .link_close     = _on_link_close,
    .complete       = _on_prov_complete,
    .reset          = _on_reset,
};

static char _stack_mesh[NIMBLE_MESH_STACKSIZE];

static void *_mesh_thread(void *arg)
{
    mesh_adv_thread(arg);
    return NULL;
}

static void _on_btn_irq(void *arg)
{
    event_post(&_btn_evevtq, &_btn_event[(unsigned)arg].super);
}

static void _on_btn_evt(void *arg)
{
    unsigned num = (unsigned)arg;
    uint8_t state = (~num & 0x01);  /* button 1 and 3 are on, the others off */
    struct bt_mesh_model *model = _btn_models[num];

    // gpio_irq_disable(_btn[num]);
    // xtimer_set(&_debounce[num], DEBOUNCE_DELAY);

    printf("Button %u was pressed\n", num);

    if (_is_provisioned && model->pub->addr != BT_MESH_ADDR_UNASSIGNED) {
        bt_mesh_model_msg_init(model->pub->msg, BT_MESH_MODEL_OP_2(0x82, 0x02));
        net_buf_simple_add_u8(model->pub->msg, state);
        net_buf_simple_add_u8(model->pub->msg, _trans_id++);
        int res = bt_mesh_model_publish(model);
        assert(res == 0);
        (void)res;
        printf("-> button state change (%u) was published\n", (unsigned)state);
    }
}

int main(void)
{
    int res;
    (void)res;
    ble_addr_t addr;

    puts("NimBLE Mesh Example");

    event_queue_init(&_btn_evevtq);
    for (unsigned i = 0; i < (sizeof(_btn_event) / sizeof(_btn_event[0])); i++) {
        event_callback_init(&_btn_event[i], _on_btn_evt, (void *)i);
    }

    /* initialize buttons and LEDs */
    for (unsigned i = 0; i < (sizeof(_led) / sizeof(_led[0])); i++) {
        gpio_init(_led[i], GPIO_OUT);
        gpio_set(_led[i]);
    }
    for (unsigned i = 0; i < (sizeof(_debounce) / sizeof(_debounce[0])); i++) {
        _debounce[i].callback = _on_debounce;
        _debounce[i].arg = (void *)i;
    }
    for (unsigned i = 0; i < (sizeof(_btn) / sizeof(_btn[0])); i++) {
        gpio_init_int(_btn[i], GPIO_IN_PU, GPIO_FALLING, _on_btn_irq, (void *)i);
    }

    /* generate and set non-resolvable private address */
    res = ble_hs_id_gen_rnd(1, &addr);
    assert(res == 0);
    res = ble_hs_id_set_rnd(addr.val);
    assert(res == 0);

    /* initialize the health publish service */
    _health_pub.msg = BT_MESH_HEALTH_FAULT_MSG(0);

    for (unsigned i = 0; i < (sizeof(_s_pub) / sizeof(_s_pub[0])); i++) {
        _s_pub[i].msg = NET_BUF_SIMPLE(2 + 2);
    }

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

    event_loop(&_btn_evevtq);
    return 0;
}
