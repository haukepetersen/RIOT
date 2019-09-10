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
#include <stdlib.h>

#include "thread.h"
#include "shell.h"
#include "random.h"
#include "nimble_riot.h"
#include "event/callback.h"

#include "host/mystats.h"
#include "host/myfilter.h"

#include "host/ble_hs.h"
#include "mesh/glue.h"
#include "mesh/porting.h"
#include "mesh/access.h"
#include "mesh/main.h"
#include "mesh/cfg_srv.h"
#include "host/mystats.h"

#include "luid.h"
#include "mesh/cfg_cli.h"

#define EXP_INTERVAL            (1U * US_PER_SEC)   /* default: 1 pkt per sec */
#define EXP_JITTER              (500U * US_PER_MS)  /* default: .5 sec jitter */
#define EXP_REPEAT              (100U)              /* default: 100 packets */

#define VENDOR_CID              0x2342              /* random... */

#define OP_GET                  BT_MESH_MODEL_OP_2(0x82, 0x01)
#define OP_SET_ACKED            BT_MESH_MODEL_OP_2(0x82, 0x02)
#define OP_SET_UNACK            BT_MESH_MODEL_OP_2(0x82, 0x03)
#define OP_STATUS               BT_MESH_MODEL_OP_2(0x82, 0x04)

#define OP_LVL_GET              BT_MESH_MODEL_OP_2(0x82, 0x05)
#define OP_LVL_SET              BT_MESH_MODEL_OP_2(0x82, 0x06)
#define OP_LVL_SET_UNACK        BT_MESH_MODEL_OP_2(0x82, 0x07)
#define OP_LVL_STATUS           BT_MESH_MODEL_OP_2(0x82, 0x08)

/* shell thread env */
static char _stack_mesh[NIMBLE_MESH_STACKSIZE];

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

#define ADDR_SERVER             (_addr_node + 1)
#define ADDR_CLIENT             (_addr_node + 2)

static const uint8_t _key_net[16] = PROV_KEY_NET;
static const uint8_t _key_app[16] = PROV_KEY_APP;
static uint8_t _key_dev[16];
static uint16_t _addr_node;

static uint8_t _trans_id = 0;
static int _is_provisioned = 0;

static struct bt_mesh_cfg_srv _cfg_srv = {
    .relay = BT_MESH_RELAY_ENABLED,
    .beacon = BT_MESH_BEACON_DISABLED,
    .frnd = BT_MESH_FRIEND_NOT_SUPPORTED,
    .gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
    .default_ttl = 7,

    /* 5 transmissions with 20ms interval */
    .net_transmit = BT_MESH_TRANSMIT(4, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(4, 20),
};

static struct bt_mesh_cfg_cli _cfg_cli = {
};

static struct bt_mesh_model _models_root[] = {
    BT_MESH_MODEL_CFG_SRV(&_cfg_srv),
    BT_MESH_MODEL_CFG_CLI(&_cfg_cli),
};

static void _op_lvl_get(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct os_mbuf *buf)
{
    (void)model;
    (void)ctx;
    (void)buf;
    mystats_inc_rx_app("lvl_get", 0);
}

static void _op_lvl_set(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct os_mbuf *buf)
{
    (void)model;
    (void)ctx;
    unsigned level = (unsigned)net_buf_simple_pull_le16(buf);
    mystats_inc_rx_app("lvl_set", level);
}

static void _op_lvl_set_unack(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct os_mbuf *buf)
{
    (void)model;
    (void)ctx;
    unsigned level = (unsigned)net_buf_simple_pull_le16(buf);
    mystats_inc_rx_app("lvl_set_unack", level);
}

static void _op_lvl_status(struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct os_mbuf *buf)
{
    (void)model;
    (void)ctx;
    unsigned level = (unsigned)net_buf_simple_pull_le16(buf);
    mystats_inc_rx_app("lvl_status", level);
}

static void _send_status(struct bt_mesh_model *model,
                         struct bt_mesh_msg_ctx *ctx,
                         struct os_mbuf *buf)
{
    (void)buf;

    mystats_inc_tx_app("status", (unsigned)_trans_id++);
    struct os_mbuf *msg = NET_BUF_SIMPLE(2 + 1 + 4);
    bt_mesh_model_msg_init(msg, OP_STATUS);
    net_buf_simple_add_u8(msg, _trans_id);  /* intentional missusage here */
    int res = bt_mesh_model_send(model, ctx, msg, NULL, NULL);
    assert(res == 0);
    (void)res;
    os_mbuf_free_chain(msg);
}

static void _op_get(struct bt_mesh_model *model,
                    struct bt_mesh_msg_ctx *ctx,
                    struct os_mbuf *buf)
{
    mystats_inc_rx_app("get", (unsigned)buf->om_data[1]);
    _send_status(model, ctx, buf);
}

static void _op_set_unack(struct bt_mesh_model *model,
                          struct bt_mesh_msg_ctx *ctx,
                          struct os_mbuf *buf)
{
    (void)model;
    (void)ctx;
    mystats_inc_rx_app("set_unack", (unsigned)buf->om_data[1]);
    // printf("OP_SET_UNACK val %i, tid %i\n",
           // (int)buf->om_data[0], (int)buf->om_data[1]);
}

static void _op_set(struct bt_mesh_model *model,
                              struct bt_mesh_msg_ctx *ctx,
                              struct os_mbuf *buf)
{
    // printf("OP_SET val %i, tid %i\n",
           // (int)buf->om_data[0], (int)buf->om_data[1]);
    mystats_inc_rx_app("set", (unsigned)buf->om_data[1]);
    _send_status(model, ctx, buf);
}

static void _op_status(struct bt_mesh_model *model,
                       struct bt_mesh_msg_ctx *ctx,
                       struct os_mbuf *buf)
{
    (void)model;
    (void)ctx;
    mystats_inc_rx_app("stats", (unsigned)buf->om_data[0]);
    // printf("OP_STATUS tid %i\n", (int)buf->om_data[0]);
}

static const struct bt_mesh_model_op _lvl_svr_op[] = {
    { OP_LVL_GET, 0, _op_lvl_get },
    { OP_LVL_SET, 3, _op_lvl_set },
    { OP_LVL_SET_UNACK, 3, _op_lvl_set_unack },
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op _lvl_cli_op[] = {
    { OP_LVL_STATUS, 0, _op_lvl_status },
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op _led_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x01), 0, _op_get },
    { BT_MESH_MODEL_OP_2(0x82, 0x02), 2, _op_set },
    { BT_MESH_MODEL_OP_2(0x82, 0x03), 2, _op_set_unack },
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op _btn_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x04), 1, _op_status },
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model_pub _s_pub[4];

static struct bt_mesh_model _models_svr[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, _led_op,
                  &_s_pub[0], (void *)0),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_LEVEL_SRV, _lvl_svr_op,
                  &_s_pub[1], (void *)0),
};

static struct bt_mesh_model _models_cli[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, _btn_op,
                  &_s_pub[2], (void *)0),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_LEVEL_CLI, _lvl_cli_op,
                  &_s_pub[3], (void *)0),
};

static struct bt_mesh_elem _elements[] = {
    BT_MESH_ELEM(0, _models_root, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_svr, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, _models_cli, BT_MESH_MODEL_NONE),
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

static void _prov_base(void)
{
    puts("Provisioning the base device:");
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

    /* do general device provisioning */
    int res = bt_mesh_provision(_key_net, PROV_NET_IDX, PROV_FLAGS,
                                PROV_IV_INDEX, _addr_node, _key_dev);
    assert(res == 0);
    /* add our app key to the node */
    res = bt_mesh_cfg_app_key_add(PROV_NET_IDX, _addr_node, PROV_NET_IDX,
                                  PROV_APP_IDX, _key_app, NULL);
    assert(res == 0);

    puts("Base provisioning done\n");
}

static void _prov_source(void)
{
    int res;

    struct bt_mesh_cfg_mod_pub pub = {
        .addr = PROV_ADDR_GROUP0,
        .app_idx = PROV_APP_IDX,
        .ttl = 15,
        .transmit = 0,
    };

    puts("Provisioning the SOURCE element:");
    res = bt_mesh_cfg_mod_app_bind(PROV_NET_IDX, _addr_node, ADDR_CLIENT,
                                   PROV_APP_IDX,
                                   BT_MESH_MODEL_ID_GEN_ONOFF_CLI, NULL);
    assert(res == 0);
    res = bt_mesh_cfg_mod_app_bind(PROV_NET_IDX, _addr_node, ADDR_CLIENT,
                                   PROV_APP_IDX,
                                   BT_MESH_MODEL_ID_GEN_LEVEL_CLI, NULL);
    assert(res == 0);
    res = bt_mesh_cfg_mod_pub_set(PROV_NET_IDX, _addr_node, ADDR_CLIENT,
                                  BT_MESH_MODEL_ID_GEN_ONOFF_CLI,
                                  &pub, NULL);
    assert(res == 0);
    res = bt_mesh_cfg_mod_pub_set(PROV_NET_IDX, _addr_node, ADDR_CLIENT,
                                  BT_MESH_MODEL_ID_GEN_LEVEL_CLI,
                                  &pub, NULL);
    assert(res == 0);
    puts("SOURCE element provisioned");

    mystats_clear();
    mystats_enable();
}

static void _prov_sink(void)
{
    int res;

    puts("Provisioning the SINK element:");
    res = bt_mesh_cfg_mod_app_bind(PROV_NET_IDX, _addr_node, ADDR_SERVER,
                                   PROV_APP_IDX,
                                   BT_MESH_MODEL_ID_GEN_ONOFF_SRV, NULL);
    assert(res == 0);
    res = bt_mesh_cfg_mod_app_bind(PROV_NET_IDX, _addr_node, ADDR_SERVER,
                                   PROV_APP_IDX,
                                   BT_MESH_MODEL_ID_GEN_LEVEL_SRV, NULL);
    assert(res == 0);
    res = bt_mesh_cfg_mod_sub_add(PROV_NET_IDX, _addr_node, ADDR_SERVER,
                                  PROV_ADDR_GROUP0,
                                  BT_MESH_MODEL_ID_GEN_ONOFF_SRV, NULL);
    assert(res == 0);
    res = bt_mesh_cfg_mod_sub_add(PROV_NET_IDX, _addr_node, ADDR_SERVER,
                                  PROV_ADDR_GROUP0,
                                  BT_MESH_MODEL_ID_GEN_LEVEL_SRV, NULL);
    assert(res == 0);
    puts("SINK element provisioned");

    mystats_clear();
    mystats_enable();
}

static int _cmd_clear(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    mystats_clear();
    return 0;
}

static int _cmd_stats(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    mystats_dump();
    return 0;
}

static int _cmd_cfg_source(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    _prov_source();
    return 0;
}

static int _cmd_cfg_sink(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    _prov_sink();
    return 0;
}

static int _cmd_wl(int argc, char **argv)
{
    if (argc < 2) {
        puts("err: whitelist command missing parameter");
        assert(0);
    }

    int res = myfilter_add_str(argv[1]);
    assert(res == 0);
    printf("whitelist: added %s\n", argv[1]);

    return 0;
}

static int _cmd_run(int argc, char **argv)
{
    uint32_t itvl = EXP_INTERVAL;
    unsigned cnt = EXP_REPEAT;
    struct bt_mesh_model *model = &_models_cli[0];

    if (!_is_provisioned || (model->pub->addr == BT_MESH_ADDR_UNASSIGNED)) {
        puts("err: node or element not provisioned");
        return 1;
    }

    if (argc >= 2) {
        cnt = (unsigned)atoi(argv[1]);
    }
    if (argc >= 3) {
        itvl = (uint32_t)atoi(argv[2]);
    }

    xtimer_ticks32_t last_wakeup = xtimer_now();
    _trans_id = 0;  /* reset, this way we can trace the experiment */

    for (unsigned i = 0; i < cnt; i++) {
        // printf("publishing event %u\n", i);

        mystats_inc_tx_app("pub", _trans_id);
        bt_mesh_model_msg_init(model->pub->msg, OP_SET_UNACK);
        net_buf_simple_add_u8(model->pub->msg, 0);
        net_buf_simple_add_u8(model->pub->msg, _trans_id++);
        int res = bt_mesh_model_publish(model);
        assert(res == 0);
        (void)res;

            /* REMOVE again */
            extern u8_t bt_mesh_net_transmit_get(void);
            extern u8_t bt_mesh_relay_retransmit_get(void);
            extern u8_t bt_mesh_relay_get(void);
            uint8_t trans = bt_mesh_net_transmit_get();
            printf("NETWORK TRANSMIT STATE: 0x%02x -> cnt %i, int: %i\n",
                   (int)trans, (int)(trans >> 5), (int)(trans & 0x1f));
            uint8_t relay = bt_mesh_relay_retransmit_get();
            uint8_t st = bt_mesh_relay_get();
            printf("RELAY RETRANSMIT STATE: 0x%02x -> cnt %i, int: %i\n",
                    (int)st, (int)(relay >> 5), (int)(relay & 0x1f));

        xtimer_periodic_wakeup(&last_wakeup, itvl);
    }

    puts("EXP DONE");

    return 0;
}

static int _cmd_run_lvl(int argc, char **argv)
{
    uint32_t itvl = EXP_INTERVAL;
    uint32_t jttr = EXP_JITTER;
    unsigned cnt = EXP_REPEAT;
    struct bt_mesh_model *model = &_models_cli[1];

    if (!_is_provisioned || (model->pub->addr == BT_MESH_ADDR_UNASSIGNED)) {
        puts("err: node or element not provisioned");
        return 1;
    }

    if (argc >= 2) {
        cnt = (unsigned)atoi(argv[1]);
    }
    if (argc >= 3) {
        itvl = (uint32_t)atoi(argv[2]);
    }
    if (argc >= 4) {
        jttr = (uint32_t)atoi(argv[3]);
    }

    uint32_t min = itvl - jttr;
    uint32_t max = itvl + jttr;
    assert(min < max);

    _trans_id = 0;  /* reset, this way we can trace the experiment */

    for (unsigned i = 0; i < cnt; i++) {
        mystats_inc_tx_app("pub_lvl", (_trans_id + _addr_node));
        bt_mesh_model_msg_init(model->pub->msg, OP_LVL_SET_UNACK);
        net_buf_simple_add_le16(model->pub->msg, (_trans_id + _addr_node));
        net_buf_simple_add_u8(model->pub->msg, _trans_id++);
        int res = bt_mesh_model_publish(model);
        assert(res == 0);
        (void)res;

        xtimer_usleep(random_uint32_range(min, max));
    }

    puts("EXP DONE");

    return 0;
}

static const shell_command_t _shell_cmds[] = {
    { "clr", "reset stats", _cmd_clear },
    { "stats", "show stats", _cmd_stats },
    { "cfg_source", "provision node as source", _cmd_cfg_source },
    { "cfg_sink", "provision node as sink", _cmd_cfg_sink },
    { "wl", "white list address", _cmd_wl },
    { "run", "run the experiment", _cmd_run },
    { "run_lvl", "run exp, use level model", _cmd_run_lvl },
    { NULL, NULL, NULL }
};

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
    // ble_addr_t addr;

    puts("ICN-BLE experiment: 1-to-many setunack");

    /* generate and set non-resolvable private address */
    // TODO: make this static!
    // res = ble_hs_id_gen_rnd(1, &addr);
    // assert(res == 0);
    // res = ble_hs_id_set_rnd(addr.val);
    // assert(res == 0);

    for (unsigned i = 0; i < (sizeof(_s_pub) / sizeof(_s_pub[0])); i++) {
        _s_pub[i].msg = NET_BUF_SIMPLE(2 + 4);
    }

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

    _prov_base();

    /* start the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(_shell_cmds, line_buf, sizeof(line_buf));

    return 0;
}
