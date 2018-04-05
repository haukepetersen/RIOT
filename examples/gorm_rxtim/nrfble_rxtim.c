/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_nrf5x_nrfble
 *
 * @note        This driver is not thread safe and should only be used by a
 *              single thread at once!
 * @{
 *
 * @file
 * @brief       Bluetooth low energy radio driver for nRF5x SoCs
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <errno.h>

#include "cpu.h"
#include "assert.h"

#include "nrfble.h"
#include "net/netdev/ble.h"

#define ENABLE_DEBUG            (0)
#include "debug.h"

/**
 * @brief   Driver specific device configuration
 * @{
 */
#define CONF_MODE               RADIO_MODE_MODE_Ble_1Mbit
#define CONF_LEN                (8U)
#define CONF_S0                 (1U)
#define CONF_S1                 (0U)
#define CONF_STATLEN            (0U)
#define CONF_BASE_ADDR_LEN      (3U)
#define CONF_ENDIAN             RADIO_PCNF1_ENDIAN_Little
#define CONF_WHITENING          RADIO_PCNF1_WHITEEN_Enabled
#define CONF_CRC_LEN            (0X3 | RADIO_CRCCNF_SKIPADDR_Msk)
#define CONF_CRC_POLY           (0x00065b)
#define CONF_CRC_INIT           (0x555555)
/** @} */

/**
 * @brief   Allocate the netdev device descriptor
 */
netdev_t nrfble_dev;

/**
 * @brief   Remember the radio context when receiving things
 */
netdev_ble_ctx_t *rx_ctx;

/**
 * @brief   Map logical BLE channel values to actual radio frequencies
 */
static const uint8_t ble_chan_map[40] = {
    [ 0] = 4,
    [ 1] = 6,
    [ 2] = 8,
    [ 3] = 10,
    [ 4] = 12,
    [ 5] = 14,
    [ 6] = 16,
    [ 7] = 18,
    [ 8] = 20,
    [ 9] = 22,
    [10] = 24,
    [11] = 28,
    [12] = 30,
    [13] = 32,
    [14] = 34,
    [15] = 36,
    [16] = 38,
    [17] = 40,
    [18] = 42,
    [19] = 44,
    [20] = 46,
    [21] = 48,
    [22] = 50,
    [23] = 52,
    [24] = 54,
    [25] = 56,
    [26] = 58,
    [27] = 60,
    [28] = 62,
    [29] = 64,
    [30] = 66,
    [31] = 68,
    [32] = 70,
    [33] = 72,
    [34] = 74,
    [35] = 76,
    [36] = 78,
    [37] = 2,
    [38] = 26,
    [39] = 80,
};

/**
 * @brief   Set radio into idle (DISABLED) state
 */
static void go_idle(void)
{
    /* only do anything in case RX is active */
    if (NRF_RADIO->INTENSET & RADIO_INTENSET_DISABLED_Msk) {
        NRF_RADIO->INTENCLR = RADIO_INTENCLR_DISABLED_Msk;
        /* set device into basic disabled state */
        NRF_RADIO->EVENTS_DISABLED = 0;
        NRF_RADIO->TASKS_DISABLE = 1;
        while (NRF_RADIO->EVENTS_DISABLED == 0) {
            /* TODO: enable sleep here */
            // cortexm_sleep_until_event();
        }
    }
}

static void set_context(netdev_ble_ctx_t *ctx)
{
    assert(ctx->chan <= NRFBLE_CHAN_MAX);

    NRF_RADIO->FREQUENCY = ble_chan_map[ctx->chan];
    NRF_RADIO->PREFIX0 = ctx->aa.raw[3];
    NRF_RADIO->BASE0   = (uint32_t)((ctx->aa.raw[0] << 8) |
                                    (ctx->aa.raw[1] << 16) |
                                    (ctx->aa.raw[2] << 24));
    NRF_RADIO->DATAWHITEIV = ctx->chan;
    NRF_RADIO->CRCINIT = ctx->crc;
}

static int16_t nrfble_get_txpower(void)
{
    int8_t p = (int8_t)NRF_RADIO->TXPOWER;
    if (p < 0) {
        return (int16_t)(0xff00 | p);
    }
    return (int16_t)p;
}

static void nrfble_set_txpower(int16_t power)
{
    if (power > 2) {
        NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Pos4dBm;
    }
    else if (power > -2) {
        NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_0dBm;
    }
    else if (power > -6) {
        NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg4dBm;
    }
    else if (power > -10) {
        NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg8dBm;
    }
    else if (power > -14) {
        NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg12dBm;
    }
    else if (power > -18) {
        NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg16dBm;
    }
    else if (power > -25) {
        NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg20dBm;
    }
    else {
        NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg30dBm;
    }
}


/**
 * @brief   Radio interrupt routine
 */
void isr_radio(void)
{
    if (NRF_RADIO->EVENTS_DISABLED == 1) {
        NRF_RADIO->EVENTS_DISABLED = 0;

        rx_ctx->crc = NRF_RADIO->CRCSTATUS;
        nrfble_dev.event_callback(&nrfble_dev, NETDEV_EVENT_RX_COMPLETE);
    }
    cortexm_isr_end();
}


netdev_t *nrfble_setup(void)
{
    nrfble_dev.driver = &nrfble_netdev;
    nrfble_dev.event_callback = NULL;
    nrfble_dev.context = NULL;
#ifdef MODULE_NETSTATS_L2
    memset(&nrfble_dev.stats, 0, sizeof(netstats_t));;
#endif
    return &nrfble_dev;
}

static int nrfble_init(netdev_t *dev)
{
    (void)dev;
    assert(nrfble_dev.driver && nrfble_dev.event_callback);

    /* power on the NRFs radio */
    NRF_RADIO->POWER = 1;
    /* configure variable parameters to default values */
    NRF_RADIO->TXPOWER = NRFBLE_TXPOWER_DEFAULT;
    /* always send from and listen to logical address 0 */
    NRF_RADIO->TXADDRESS = 0x00UL;
    NRF_RADIO->RXADDRESSES = 0x01UL;
    /* load driver specific configuration */
    NRF_RADIO->MODE = CONF_MODE;
    /* configure data fields and packet length whitening and endianess */
    NRF_RADIO->PCNF0 = ((CONF_S1 << RADIO_PCNF0_S1LEN_Pos) |
                        (CONF_S0 << RADIO_PCNF0_S0LEN_Pos) |
                        (CONF_LEN << RADIO_PCNF0_LFLEN_Pos));
    NRF_RADIO->PCNF1 = ((CONF_WHITENING << RADIO_PCNF1_WHITEEN_Pos) |
                        (CONF_ENDIAN << RADIO_PCNF1_ENDIAN_Pos) |
                        (CONF_BASE_ADDR_LEN << RADIO_PCNF1_BALEN_Pos) |
                        (CONF_STATLEN << RADIO_PCNF1_STATLEN_Pos) |
                        (NETDEV_BLE_PDU_MAXLEN << RADIO_PCNF1_MAXLEN_Pos));
    /* configure the CRC unit, we skip the address field as this seems to lead
     * to wrong checksum calculation on nRF52 devices in some cases */
    NRF_RADIO->CRCCNF = CONF_CRC_LEN;
    NRF_RADIO->CRCPOLY = CONF_CRC_POLY;
    NRF_RADIO->CRCINIT = CONF_CRC_INIT;
    /* set shortcuts for more efficient transfer */
    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk);
    /* enable global radio interrupts, but leave local interrupts masked */
    NRF_RADIO->INTENCLR = 0xffffffff;
    NVIC_EnableIRQ(RADIO_IRQn);

    DEBUG("[nrfble] initialization successful\n");
    return 0;
}

static int nrfble_send(netdev_t *dev, const struct iovec *data, unsigned count)
{
    (void)dev;
    (void)count;

    assert(count == 2);
    assert((data != NULL) &&
           (data[0].iov_len == sizeof(netdev_ble_ctx_t)) &&
           (data[1].iov_len == sizeof(netdev_ble_pkt_t)));

    /* cancel any RX operation (if ongoing) */
    go_idle();

    /* get the BLE pkt data */
    netdev_ble_ctx_t *ctx = data[0].iov_base;
    netdev_ble_pkt_t *pkt = data[1].iov_base;

    DEBUG("[nrfble] send: sending %i bytes on chan %i\n",
          (int)pkt->len, (int)ctx->chan);

    /* setup radio */
    set_context(ctx);
    NRF_RADIO->PACKETPTR = (uint32_t)(pkt);

    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_TXEN = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0) {
        /* TODO: enable (and verify) sleep below */
        // cortexm_sleep_until_event();
    }

    return (int)pkt->len;
}

static int nrfble_recv(netdev_t *dev, void *buf, size_t len, void *info)
{
    (void)dev;
    (void)len;

    /* cancel any ongoing operation */
    go_idle();

    if (buf) {
        assert(info);
        assert(len == sizeof(netdev_ble_pkt_t));

        rx_ctx = (netdev_ble_ctx_t *)info;

        /* setup radio */
        set_context(rx_ctx);
        NRF_RADIO->PACKETPTR = (uint32_t)(buf);
        NRF_RADIO->INTENSET = RADIO_INTENSET_DISABLED_Msk;

        /* put radio into RX mode */
        NRF_RADIO->EVENTS_DISABLED = 0;
        NRF_RADIO->TASKS_RXEN = 1;
        DEBUG("[nrfble] recv: now in RX mode (chan %i)\n", (int)rx_ctx->chan);
    }

    return 0;
}

static void nrfble_isr(netdev_t *dev)
{
    if (nrfble_dev.event_callback) {
        nrfble_dev.event_callback(dev, NETDEV_EVENT_RX_COMPLETE);
    }
}

static int nrfble_get(netdev_t *dev, netopt_t opt, void *val, size_t max_len)
{
    (void)dev;
    (void)max_len;

    switch (opt) {
        case NETOPT_TX_POWER:
            assert(max_len >= sizeof(int16_t));
            *((int16_t *)val) = nrfble_get_txpower();
            return sizeof(int16_t);
        case NETOPT_DEVICE_TYPE:
            assert(max_len >= sizeof(uint16_t));
            *((uint16_t *)val) = NETDEV_TYPE_BLE;
            return sizeof(uint16_t);
        default:
            return -ENOTSUP;
    }
}

static int nrfble_set(netdev_t *dev, netopt_t opt, const void *val, size_t len)
{
    (void)dev;
    (void)len;

    switch (opt) {
        case NETOPT_TX_POWER:
            assert(len == sizeof(int16_t));
            nrfble_set_txpower(*((const int16_t *)val));
            return sizeof(int16_t);
        default:
            return -ENOTSUP;
    }
}

/**
 * @brief   Export of the netdev interface
 */
const netdev_driver_t nrfble_netdev = {
    .send = nrfble_send,
    .recv = nrfble_recv,
    .init = nrfble_init,
    .isr  = nrfble_isr,
    .get  = nrfble_get,
    .set  = nrfble_set
};
