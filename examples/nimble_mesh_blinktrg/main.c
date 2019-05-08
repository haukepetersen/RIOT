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
#include "shell.h"
#include "nimble_riot.h"
#include "periph/gpio.h"
#include "event/callback.h"
#include "event/timeout.h"

#include "host/mystats.h"

#include "host/ble_hs.h"
#include "mesh/glue.h"
#include "mesh/porting.h"
#include "mesh/access.h"
#include "mesh/main.h"
#include "mesh/cfg_srv.h"

#ifdef PROV_STATIC
#include "luid.h"
#include "mesh/cfg_cli.h"
#endif

#define VENDOR_CID              0x2342              /* random... */
#define DEBOUNCE_DELAY          (250 * US_PER_MS)   /* 250ms */
#define BLINK_DELAY             (1 * US_PER_SEC)    /* 1s */

// #define SET_OP                  BT_MESH_MODEL_OP_2(0x82, 0x02)  /* set */
#define SET_OP                  BT_MESH_MODEL_OP_2(0x82, 0x03)    /* unack */

/* shell thread env */
#define PRIO                    (THREAD_PRIORITY_MAIN + 1)
static char _stack_mesh[NIMBLE_MESH_STACKSIZE];
static char _stack_shell[THREAD_STACKSIZE_MAIN];

#ifdef PROV_STATIC
#define PROV_KEY_NET            { 0x23, 0x42, 0x17, 0xaf, \
                                  0x23, 0x42, 0x17, 0xaf, \
                                  0x23, 0x42, 0x17, 0xaf, \
                                  0x23, 0x42, 0x17, 0xaf }
#define PROV_KEY_APP            { 0x19, 0x28, 0x37, 0x46, \
                                  0x19, 0x28, 0x37, 0x46, \
                                  0x19, 0x28, 0x37, 0x46, \
                                  0x19, 0x28, 0x37, 0x46 }
#define PROV_NET_IDX            (0U)
#define PROV_APP_IDX            (0U)
#define PROV_IV_INDEX           (0U)
#define PROV_FLAGS              (0U)

#define PROV_ADDR_GROUP0        (0xc001)

#define ADDR_LED                (_addr_node + 1)
#define ADDR_BTN                (_addr_node + 2)

static const uint8_t _key_net[16] = PROV_KEY_NET;
static const uint8_t _key_app[16] = PROV_KEY_APP;
static uint8_t _key_dev[16];
static uint16_t _addr_node;
#endif

/* button and pin maps */
static const gpio_t _btn[] = { BTN0_PIN, BTN1_PIN, BTN2_PIN, BTN3_PIN };
static const gpio_t _led[] = { LED0_PIN, LED1_PIN, LED2_PIN, LED3_PIN };
static xtimer_t _debounce[4];
static event_queue_t _eventq;
static event_callback_t _btn_event[4];
static event_t _blink_event_base;
static event_timeout_t _blink_event;

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
    printf("LED%i - %i\n", (int)pin, (int)state);
}

static struct bt_mesh_cfg_srv _cfg_srv = {
    .relay = BT_MESH_RELAY_ENABLED,
    .beacon = BT_MESH_BEACON_DISABLED,
    .frnd = BT_MESH_FRIEND_NOT_SUPPORTED,
    .gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
    .default_ttl = 7,

    /* 3 transmissions with 20ms interval */
    .net_transmit = BT_MESH_TRANSMIT(2, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};

static struct bt_mesh_cfg_cli _cfg_cli = {
};

static struct bt_mesh_model _models_root[] = {
    BT_MESH_MODEL_CFG_SRV(&_cfg_srv),
    BT_MESH_MODEL_CFG_CLI(&_cfg_cli),
};

static void _led_op_get(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct os_mbuf *buf)
{
    (void)buf;
    struct os_mbuf *msg = NET_BUF_SIMPLE(2 + 1 + 4);
    uint8_t state = _led_read(2);

    // printf("OP: LED get -> %i\n", (int)state);

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
    (void)model;
    // uint8_t state = _led_read(2);
    uint8_t new_state = net_buf_simple_pull_u8(buf);

    // printf("OP: LED set unack -> %i\n", (int)new_state);
    _led_write(2, new_state);
    _led_write(3, new_state);

    // if ((state != new_state) && (model->pub->addr != BT_MESH_ADDR_UNASSIGNED)) {
    //     printf("  : set unack: publish state change\n");
    //     struct os_mbuf *msg = model->pub->msg;
    //     bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_2(0x82, 0x04));
    //     net_buf_simple_add_u8(msg, new_state);
    //     int res = bt_mesh_model_publish(model);
    //     assert(res == 0);
    // }
}

static void _led_op_set(struct bt_mesh_model *model,
                              struct bt_mesh_msg_ctx *ctx,
                              struct os_mbuf *buf)
{
    // printf("OP: LED set\n");
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

static const struct bt_mesh_model_op _led_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x01), 0, _led_op_get },
    { BT_MESH_MODEL_OP_2(0x82, 0x02), 2, _led_op_set },
    { BT_MESH_MODEL_OP_2(0x82, 0x03), 2, _led_op_set_unack },
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op _btn_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x04), 1, _btn_status },
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model_pub _s_pub[2];


static struct bt_mesh_model _models_led[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, _led_op,
                  &_s_pub[0], (void *)0),
};

static struct bt_mesh_model _models_btn[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, _btn_op,
                  &_s_pub[1], (void *)0),
};

static struct bt_mesh_elem _elements[] = {
    BT_MESH_ELEM(0, _models_root, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_led, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_btn, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp _node_comp = {
    .cid = VENDOR_CID,
    .elem = _elements,
    .elem_count = ARRAY_SIZE(_elements),
};

static void _on_prov_complete(uint16_t net_idx, uint16_t addr)
{
    (void)net_idx;
    (void)addr;
    _is_provisioned = 1;
    printf("Node provisioning complete!\n");
}

static const uint8_t dev_uuid[16] = MYNEWT_VAL(BLE_MESH_DEV_UUID);

static const struct bt_mesh_prov _prov_cfg = {
    .uuid           = dev_uuid,
    .output_size    = 0,
    .output_actions = 0,
    .complete       = _on_prov_complete,
};

static void _dump_key(const uint8_t *key)
{
    for (unsigned i = 0; i < 15; i++) {
        printf("%02x:", (int)key[i]);
    }
    printf("%02x", (int)key[15]);
}

extern uint8_t bt_mesh_relay_retransmit_get(void);
extern uint8_t bt_mesh_relay_get(void);
extern uint8_t bt_mesh_net_transmit_get(void);

static void _prov_static(void)
{
    puts(" -> applying static device provisioning parameters:");
    /* generate node address and device key */
    luid_get(_key_dev, 16);
    luid_get(&_addr_node, 2);
    _addr_node &= ~0x8000;      /* first bit must be 0 for unicast addresses */

    /* dump provisioning info */
    printf("  node addr: %u (0x%04x)\n", (unsigned)_addr_node, (int)_addr_node);
    printf("  IV_INDEX: %i NET_IDX: %i APP_IDX: %i",
           (int)PROV_IV_INDEX, (int)PROV_NET_IDX, (int)PROV_APP_IDX);
    printf("\n  dev key: ");
    _dump_key(_key_dev);
    printf("\n  net key: ");
    _dump_key(_key_net);
    printf("\n  app key: ");
    _dump_key(_key_app);
    puts("");

    struct bt_mesh_cfg_mod_pub pub = {
        .addr = PROV_ADDR_GROUP0,
        .app_idx = PROV_APP_IDX,
        .ttl = 15,
        .transmit = 0,
    };

    /* do general device provisioning */
    int res = bt_mesh_provision(_key_net, PROV_NET_IDX, PROV_FLAGS,
                                PROV_IV_INDEX, _addr_node, _key_dev);
    assert(res == 0);

    /* add our app key to the node */
    res = bt_mesh_cfg_app_key_add(PROV_NET_IDX, _addr_node, PROV_NET_IDX,
                                  PROV_APP_IDX, _key_app, NULL);
    assert(res == 0);

    /* provision btn element */
    res = bt_mesh_cfg_mod_app_bind(PROV_NET_IDX, _addr_node, ADDR_BTN,
                                   PROV_APP_IDX,
                                   BT_MESH_MODEL_ID_GEN_ONOFF_CLI, NULL);
    assert(res == 0);
    res = bt_mesh_cfg_mod_pub_set(PROV_NET_IDX, _addr_node, ADDR_BTN,
                                  BT_MESH_MODEL_ID_GEN_ONOFF_CLI,
                                  &pub, NULL);
    assert(res == 0);

    /* provision LED element */
    res = bt_mesh_cfg_mod_app_bind(PROV_NET_IDX, _addr_node, ADDR_LED,
                                   PROV_APP_IDX,
                                   BT_MESH_MODEL_ID_GEN_ONOFF_SRV, NULL);
    assert(res == 0);
    res = bt_mesh_cfg_mod_sub_add(PROV_NET_IDX, _addr_node, ADDR_LED,
                                  PROV_ADDR_GROUP0,
                                  BT_MESH_MODEL_ID_GEN_ONOFF_SRV, NULL);
    assert(res == 0);

    puts("Element Provisioning complete!\n");

    printf("relay status: %i\n", (int)bt_mesh_relay_get());

    uint8_t relay_retransmissions = bt_mesh_relay_retransmit_get();
    printf("Relay retransmissions: %i\n", (int)relay_retransmissions);

    printf("Net transmissions: %i\n", (int)bt_mesh_net_transmit_get());
}

static void _led_group_set(uint8_t state)
{
    struct bt_mesh_model *model = &_models_btn[0];

    if (_is_provisioned && model->pub->addr != BT_MESH_ADDR_UNASSIGNED) {
        bt_mesh_model_msg_init(model->pub->msg, SET_OP);
        net_buf_simple_add_u8(model->pub->msg, state);
        net_buf_simple_add_u8(model->pub->msg, _trans_id++);
        int res = bt_mesh_model_publish(model);
        assert(res == 0);
        (void)res;
    }
}

static void _on_blink_evt(event_t *evt)
{
    (void)evt;

    uint8_t state = (_led_read(2)) ? 0x00 : 0x01;
    _led_group_set(state);
    event_timeout_set(&_blink_event, BLINK_DELAY);
}

static void _blink_on(void)
{
    _led_write(0, 1);
    event_timeout_set(&_blink_event, BLINK_DELAY);
}

static void _blink_off(void)
{
    event_timeout_clear(&_blink_event);
    _led_write(0, 0);
}

static void _on_btn_irq(void *arg)
{
    event_post(&_eventq, &_btn_event[(unsigned)arg].super);
}

static void _on_btn_evt(void *arg)
{
    unsigned num = (unsigned)arg;

    /* debounce the button */
    gpio_irq_disable(_btn[num]);
    xtimer_set(&_debounce[num], DEBOUNCE_DELAY);

    printf("Button %u was pressed\n", num);

    switch (num) {
        case 0:
            _led_group_set(0x00);
            break;
        case 1:
            _led_group_set(0x01);
            break;
        case 2:
            _blink_on();
            break;
        case 3:
            _blink_off();
    }
}

static int _cmd_clear(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    mystats_clear();

    return 0;
}

static int _cmd_on(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    _on_btn_evt((void *)2);
    return 0;
}

static int _cmd_off(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    _on_btn_evt((void *)3);
    return 0;
}

static int _cmd_stats(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    mystats_t stats;
    mystats_get(&stats);

    printf("Stats:\n");
    printf("TX ADV\t\t%u\n", stats.tx_all);
    printf("TX mesh data\t%u\n", stats.tx_mesh_data);
    printf("TX mesh prov\t%u\n", stats.tx_mesh_prov);

    printf("TX mesh net send\t%u\n", stats.tx_mesh_net_send);

    printf("TX mesh transport unseg\t%u\n", stats.tx_mesh_transport_unseg);
    printf("TX mesh transport seg\t%u\n", stats.tx_mesh_transport_seg);
    printf("TX mesh transport ctl\t%u\n", stats.tx_mesh_transport_ctl);
    printf("TX mesh transport\t%u\n", stats.tx_mesh_transport);

    printf("TX mesh model send\t%u\n", stats.tx_mesh_model_send);
    printf("TX mesh model pub\t%u\n", stats.tx_mesh_model_pub);
    printf("TX mesh model retrans\t%u\n", stats.tx_mesh_model_retrans);

    printf("RX ADV\t\t%u\n", stats.rx_all);
    printf("RX non mesh\t%u\n", stats.rx_nonmesh);
    printf("RX nonconn IND\t%u\n", stats.rx_nonconn_ind);

    printf("RX cand\t%u\n", stats.rx_cand);
    printf("RX cand nolen\t%u\n", stats.rx_cand_nolen);
    printf("RX cand malformed\t%u\n", stats.rx_cand_malformed);

    printf("RX mesh nomesh\t%u\n", stats.rx_type_nomesh);
    printf("RX mesh data\t%u\n", stats.rx_mesh_data);
    printf("RX mesh prov\t%u\n", stats.rx_mesh_prov);
    printf("RX mesh beacon\t%u\n", stats.rx_mesh_beacon);

    printf("RX mesh dropped notprov: %u\n", stats.rx_mesh_dropped_notprov);

    return 0;
}

static const shell_command_t _shell_cmds[] = {
    { "clr", "reset stats", _cmd_clear },
    { "stats", "show stats", _cmd_stats },
    { "on", "turn periodic set on", _cmd_on },
    { "off", "turn periodic set off", _cmd_off },
    { NULL, NULL, NULL }
};

static void *_shell_thread(void *arg)
{
    (void)arg;
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(_shell_cmds, line_buf, sizeof(line_buf));
    return NULL;
}

/* TODO: move to sysinit (nimble_riot.c) */
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

    /* initialize local event queue and events */
    event_queue_init(&_eventq);
    _blink_event_base.handler = _on_blink_evt;
    event_timeout_init(&_blink_event, &_eventq, &_blink_event_base);
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

    for (unsigned i = 0; i < (sizeof(_s_pub) / sizeof(_s_pub[0])); i++) {
        _s_pub[i].msg = NET_BUF_SIMPLE(2 + 2);
    }

#ifndef PROV_STATIC
    /* reload the GATT server to link our added services */
    bt_mesh_register_gatt();
    ble_gatts_start();
#endif

    /* initialize the mesh stack */
    res = bt_mesh_init(addr.type, &_prov_cfg, &_node_comp);
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
    _prov_static();

    /* start shell thread */
    thread_create(_stack_shell, sizeof(_stack_shell),
                  PRIO, THREAD_CREATE_STACKTEST,
                  _shell_thread, NULL, "shell");

    event_loop(&_eventq);
    return 0;
}
