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
 * @brief       Simple Bluetooth Mesh Example
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "thread.h"
#include "shell.h"
#include "nimble_riot.h"
#include "periph/gpio.h"
#include "event/callback.h"

#include "nimble/nimble_npl.h"
#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "mesh/glue.h"
#include "mesh/porting.h"
#include "mesh/access.h"
#include "mesh/main.h"
#include "mesh/cfg_srv.h"
#include "mesh/health_srv.h"

#define VENDOR_CID              0x2342              /* random... */
#define DEBOUNCE_DELAY          (100 * US_PER_MS)   /* 100ms */

#define BTN                     BTN0_PIN
#define LED                     LED0_PIN

/* shell thread env */
#define PRIO                    (THREAD_PRIORITY_MAIN + 1)
static char _stack_mesh[NIMBLE_MESH_STACKSIZE];

/* allocate a event to fire on a button press and a debounce timer */
static xtimer_t _btn_debounce;
static struct ble_npl_event _btn_event;
static unsigned _btn_press_cnt = 0;


static uint8_t _trans_id = 0;
static int _is_provisioned = 0;


static void _on_debounce(void *arg)
{
    (void)arg;
    gpio_irq_enable(BTN);
}

static uint8_t _led_read(void)
{
    return (gpio_read(LED)) ? 0 : 1;      /* active low... */
}

static void _led_write(uint8_t state)
{
    gpio_write(LED, !state);
}

static void _led_op_get(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct os_mbuf *buf)
{
    (void)buf;
    unsigned pin = (unsigned)model->user_data;
    struct os_mbuf *msg = NET_BUF_SIMPLE(2 + 1 + 4);
    uint8_t state = _led_read();

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
    uint8_t state = _led_read();
    uint8_t new_state = net_buf_simple_pull_u8(buf);

    printf("OP: LED set unack (pin %u)\n", pin);
    _led_write(new_state);

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
    printf("OP: LED set (pin %u)\n", (unsigned)model->user_data);
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

static struct bt_mesh_model_pub _s_pub[2];

static struct bt_mesh_model _model_btn0[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, _btn_op,
                  &_s_pub[0], (void *)0),
};

static struct bt_mesh_model _model_led0[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, _led_op,
                  &_s_pub[1], (void *)0),
};

static struct bt_mesh_elem _elements[] = {
    BT_MESH_ELEM(0, _model_btn0, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _model_led0, BT_MESH_MODEL_NONE),
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
    // .output_size    = 4, //4,
    // .output_actions = BT_MESH_DISPLAY_NUMBER | BT_MESH_DISPLAY_STRING,
    .output_size = 0,
    .output_actions = 0,
    .output_number  = _on_output_number,
    .output_string  = _on_output_string,
    .input          = _on_input_required,
    .link_open      = _on_link_open,
    .link_close     = _on_link_close,
    .complete       = _on_prov_complete,
    .reset          = _on_reset,
};

static void _on_btn_irq(void *arg)
{
    (void)arg;
    ble_npl_eventq_put(nimble_port_get_dflt_eventq(), &_btn_event);
}

static void _on_btn_evt(struct ble_npl_event *ev)
{
    (void)ev;

    ++_btn_press_cnt;
    uint8_t state = (_btn_press_cnt & 0x01);
    struct bt_mesh_model *model = _model_btn0;

    gpio_irq_disable(BTN);
    xtimer_set(&_btn_debounce, DEBOUNCE_DELAY);

    printf("Button was pressed, its state is now %i\n", (int)state);

    if (_is_provisioned && model->pub->addr != BT_MESH_ADDR_UNASSIGNED) {
        uint16_t op = BT_MESH_MODEL_OP_2(0x82, 0x02);    /* set */
        bt_mesh_model_msg_init(model->pub->msg, op);
        net_buf_simple_add_u8(model->pub->msg, state);
        net_buf_simple_add_u8(model->pub->msg, _trans_id++);
        int res = bt_mesh_model_publish(model);
        assert(res == 0);
        (void)res;
        printf("-> button state change (%u) was published\n", (unsigned)state);
    }
}

static const shell_command_t _shell_cmds[] = {
    { NULL, NULL, NULL }
};

/* TODO: move to sysinit (nimble_riot.c) */
static void *_mesh_thread(void *arg)
{
    puts("STARTING MESH ADV THREAD NOW");
    mesh_adv_thread(arg);
    return NULL;
}

int main(void)
{
    int res;
    (void)res;

    puts("NimBLE Mesh Example");

    /* initialize LED */
    gpio_init(LED, GPIO_OUT);
    gpio_set(LED);

    /* initialize everything regarding the button */
    ble_npl_event_init(&_btn_event, _on_btn_evt, NULL);
    _btn_debounce.callback = _on_debounce;
    _btn_debounce.arg = NULL;
    gpio_init_int(BTN, GPIO_IN_PU, GPIO_FALLING, _on_btn_irq, NULL);

    /* allocate memory for publish messages */
    for (unsigned i = 0; i < (sizeof(_s_pub) / sizeof(_s_pub[0])); i++) {
        _s_pub[i].msg = NET_BUF_SIMPLE(2 + 2);
    }

    /* reload the GATT server to link our added services */
    bt_mesh_register_gatt();
    ble_gatts_start();

    /* initialize the mesh stack */
    res = bt_mesh_init(nimble_riot_own_addr_type, &_prov_cfg, &_node_comp);
    if (res != 0) {
        printf("err: bt_mesh_init failed (%i)\n", res);
    }
    assert(res == 0);

    /* run mesh thread */
    thread_create(_stack_mesh, sizeof(_stack_mesh),
                  NIMBLE_MESH_PRIO, THREAD_CREATE_STACKTEST,
                  _mesh_thread, NULL, "nimble_mesh");

    puts("mesh init ok");

    /* advertise this node as unprovisioned */
    res = bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
    if (res != 0) {
        printf("err: bt_mesh_prov_enable failed (%i)\n", res);
    }
    assert(res == 0);

    puts("advertising this device for provisioning now...");

    /* start shell thread */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(_shell_cmds, line_buf, sizeof(line_buf));
    return 0;
}
