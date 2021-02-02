/*
 * Copyright (C) 2018-2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_nimble_autoconn
 * @{
 *
 * @file
 * @brief       Autoconn connection manager implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "assert.h"
#include "random.h"
#include "net/bluetil/ad.h"
#include "net/bluetil/addr.h"
#include "nimble_netif.h"
#include "nimble_netif_conn.h"
#include "nimble_scanner.h"
#include "nimble_autoconn2.h"
#include "nimble_autoconn2_params.h"

#include "host/ble_hs.h"
#include "nimble/nimble_port.h"

#define ENABLE_DEBUG    0
#include "debug.h"

static const uint8_t _ad_filter[] = NIMBLE_AUTOCONN2_FILTER;

static nimble_autoconn2_params_t _cfg;
static uint8_t _ad_buf[BLE_ADV_PDU_LEN];
static uint8_t _ad_len = 0;


static void _get_conn_params(struct ble_gap_conn_params *out,
                             uint32_t itvl, uint32_t latency, uint32_t svsn_to)
{
    out->scan_itvl
    static struct ble_gap_conn_params _conn_params;
}


#if IS_ACTIVE(MODULE_NIMBLE_AUTOCONN2_HUB)
static void _on_scan_evt(uint8_t type, const ble_addr_t *addr, int8_t rssi,
                         const uint8_t *ad_buf, size_t ad_len)
{
    (void)rssi;

    /* we are only interested in ADV_IND packets, the rest can be dropped right
     * away */
    if (type != BLE_HCI_ADV_TYPE_ADV_IND) {
        return;
    }


    /* for connection checking we need the address in network byte order */
    uint8_t addrn[BLE_ADDR_LEN];
    bluetil_addr_swapped_cp(addr->val, addrn);
    bluetil_ad_t ad = {
        .buf  = (uint8_t *)ad_buf,
        .pos  = ad_len,
        .size = ad_len
    };

    if (!nimble_netif_conn_connected(addrn) &&
        bluetil_ad_find_and_cmp(&ad, _ad_filter[0],
                                &_ad_filter[1], (sizeof(_ad_filter) - 1))) {
        nimble_scanner_stop();

        if (nimble_netif_conn_count(NIMBLE_NETIF_UNUSED) > 0) {
            int res = nimble_netif_connect(addr, NULL, _cfg.conn_timeout);
            assert(res >= 0);
            (void)res;
        }
    }
}
#endif

#if IS_ACTIVE(MODULE_NIMBLE_AUTOCONN2_LEAF)
/* update the connection parameters after a connection has been established */
static void _conn_update(int handle)
{
    struct ble_gap_upd_params params;
    memset(&params, 0, sizeof(struct ble_gap_upd_params));
    params.itvl_min = BLE_GAP_CONN_ITVL_MS(_cfg.conn_itvl_ms)
    params.itvl_max = params->itvl_max;
    params.latency = _cfg.conn_latency;
    params.supervision_timeout = BLE_GAP_SUPERVISION_TIMEOUT_MS(_cfg->conn_svsn_to_ms);

    nimble_netif_update(handle, &params);
}

static void _accept(void)
{
    if (nimble_netif_conn_count(NIMBLE_NETIF_UNUSED) > 0) {
        struct ble_gap_adv_params params;
        memset(&params, 0, sizeof(struct ble_gap_adv_params));
        params.conn_mode = BLE_GAP_CONN_MODE_UND;
        params.disc_mode = BLE_GAP_DISC_MODE_NON;
        params.itvl_min = BLE_GAP_ADV_ITVL_MS(_cfg.adv_itvl_ms);
        params.itvl_max = params.itvl_min;
        nimble_Netif_accept(_ad_buf, _ad_len, )
    }
}
#endif

static void _on_netif_evt(int handle, nimble_netif_event_t event,
                          const uint8_t *addr)
{
    switch (event) {
        case NIMBLE_NETIF_ACCEPTING:
            break;
        case NIMBLE_NETIF_ACCEPT_STOP:
            break;
        case NIMBLE_NETIF_INIT_SLAVE:
            break;
        case NIMBLE_NETIF_CONNECTED_SLAVE:
            _conn_update();
            _accept();
            break;
        case NIMBLE_NETIF_CLOSED_SLAVE:
            _accept();
            break;
        case NIMBLE_NETIF_ABORT_SLAVE:
            break;

        case NIMBLE_NETIF_INIT_MASTER:
            break;
        case NIMBLE_NETIF_CONNECTED_MASTER:
            _scan_resume();
            break;
        case NIMBLE_NETIF_CLOSED_MASTER:
            break;
        case NIMBLE_NETIF_ABORT_MASTER:
            _scan_resume();
            break;
        case NIMBLE_NETIF_CONN_UPDATED:
            break;
        default:
            /* this should never happen */
            assert(0);
    }

    /* pass events to high-level user if someone registered for them */
    if (_eventcb) {
        _eventcb(handle, event, addr);
    }
}

static int _conn_update(nimble_netif_conn_t *conn, int handle, void *arg)
{
    (void)conn;
    nimble_netif_update(handle, (const struct ble_gap_upd_params *)arg);
    return 0;
}

int nimble_autoconn_init(const nimble_autoconn_params_t *params,
                         const uint8_t *ad, size_t adlen)
{
    /* register our event callback */
    nimble_netif_eventcb(_on_netif_evt);
    /* setup state machine timer (we use NimBLEs callouts for this) */
    ble_npl_callout_init(&_state_evt, nimble_port_get_dflt_eventq(),
                         _on_state_change, NULL);
    /* at last, set the given parameters */
    return nimble_autoconn_update(params, ad, adlen);
}

void nimble_autoconn_eventcb(nimble_netif_eventcb_t cb)
{
    _eventcb = cb;
}

int nimble_autoconn_update(const nimble_autoconn_params_t *params,
                           const uint8_t *ad, size_t adlen)
{
    int res;
    (void)res;

    if ((params == NULL) || (adlen > sizeof(_ad_buf)) ||
        ((adlen > 0) && (ad == NULL))) {
        return NIMBLE_AUTOCONN_PARAMERR;
    }

    /* scan and advertising period configuration */
    ble_npl_time_ms_to_ticks(params->period_adv, &_timeout_adv_period);
    ble_npl_time_ms_to_ticks(params->period_scan, &_timeout_scan_period);
    ble_npl_time_ms_to_ticks(params->period_jitter, &_period_jitter);

    /* populate the connection parameters */
    _conn_params.scan_itvl = BLE_GAP_SCAN_ITVL_MS(params->scan_win);
    _conn_params.scan_window = _conn_params.scan_itvl;
    _conn_params.itvl_min = BLE_GAP_CONN_ITVL_MS(params->conn_itvl);
    _conn_params.itvl_max = _conn_params.itvl_min;
    _conn_params.latency = 0;
    _conn_params.supervision_timeout = BLE_GAP_SUPERVISION_TIMEOUT_MS(
                                                         params->conn_super_to);
    _conn_params.min_ce_len = 0;
    _conn_params.max_ce_len = 0;
    _conn_timeout = params->conn_timeout;

    /* we use the same values to updated existing connections */
    struct ble_gap_upd_params conn_update_params;
    conn_update_params.itvl_min = _conn_params.itvl_min;
    conn_update_params.itvl_max = _conn_params.itvl_max;
    conn_update_params.latency = _conn_params.latency;
    conn_update_params.supervision_timeout = _conn_params.supervision_timeout;
    conn_update_params.min_ce_len = 0;
    conn_update_params.max_ce_len = 0;

    /* calculate the used scan parameters */
    struct ble_gap_disc_params scan_params;
    scan_params.itvl = BLE_GAP_SCAN_ITVL_MS(params->scan_itvl);
    scan_params.window = BLE_GAP_SCAN_WIN_MS(params->scan_win);
    scan_params.filter_policy = 0;
    scan_params.limited = 0;
    scan_params.passive = 0;
    scan_params.filter_duplicates = 1;

    /* set the advertising parameters used */
    _adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    _adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    _adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(params->adv_itvl);
    _adv_params.itvl_max = _adv_params.itvl_min;
    _adv_params.channel_map = 0;
    _adv_params.filter_policy = 0;
    _adv_params.high_duty_cycle = 0;

    /* initialize the advertising data that will be used */
    if (adlen > 0) {
        memcpy(_ad_buf, ad, adlen);
        bluetil_ad_init(&_ad, _ad_buf, adlen, sizeof(_ad_buf));
    }
    else {
        uint16_t svc = SVC_FILTER;
        bluetil_ad_init_with_flags(&_ad, _ad_buf, sizeof(_ad_buf),
                                   BLUETIL_AD_FLAGS_DEFAULT);
        bluetil_ad_add(&_ad, BLE_GAP_AD_UUID16_INCOMP, &svc, sizeof(svc));
        if (params->node_id) {
            res = bluetil_ad_add(&_ad, BLE_GAP_AD_NAME,
                                 params->node_id, strlen(params->node_id));
            if (res != BLUETIL_AD_OK) {
                return NIMBLE_AUTOCONN_ADERR;
            }
        }
    }

    /* initialize scanner with default parameters */
    nimble_scanner_init(&scan_params, _on_scan_evt);

    /* we also need to apply the new connection parameters to all BLE
     * connections where we are in the MASTER role */
    nimble_netif_conn_foreach(NIMBLE_NETIF_GAP_MASTER, _conn_update,
                              &conn_update_params);

    return NIMBLE_AUTOCONN_OK;
}

void nimble_autoconn_enable(void)
{
    _enabled = 1;
    _activate();
}

void nimble_autoconn_disable(void)
{
    _enabled = 0;
    _deactivate();
}
