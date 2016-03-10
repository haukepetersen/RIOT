/*
 * Copyright (C) 2015 Loci Controls Inc.
 *               2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     driver_cc2538
 * @{
 *
 * @file
 * @brief       Netdev2 adaption for the cc2538rf radio driver
 *
 * @author      Ian Martin <ian@locicontrols.com>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <errno.h>
#include <string.h>

#include "mutex.h"
#include "net/netdev2.h"

#include "cc2538rf.h"

#define ENABLE_DEBUG (0)
#include "debug.h"



// static bool addr_is_long = false;
// static uint8_t long_addr[CC2538_LONG_ADDR_LEN] = { 0 };
// static uint8_t short_addr[CC2538_SHORT_ADDR_LEN] = { 0 };

// static bool use_preloading = false;

// static netopt_state_t state = NETOPT_STATE_OFF;

// static int set_state(netopt_state_t s)
// {
//     int res = -1;

    // switch (s) {
    //     case NETOPT_STATE_OFF:
    //     case NETOPT_STATE_SLEEP:
    //         cc2538_off();
    //         state = s;
    //         res = RES_OK;
    //         break;

    //     case NETOPT_STATE_IDLE:
    //     case NETOPT_STATE_RX:
    //         if (!cc2538_is_on()) {
    //             cc2538_on();
    //             RFCORE_WAIT_UNTIL(RFCORE->XREG_FSMSTAT0bits.FSM_FFCTRL_STATE > FSM_STATE_RX_CALIBRATION);
    //         }
    //         state = s;
    //         res = RES_OK;
    //         break;

    //     case NETOPT_STATE_TX:
    //         if (!cc2538_is_on()) {
    //             cc2538_on();
    //             RFCORE_WAIT_UNTIL(RFCORE->XREG_FSMSTAT0bits.FSM_FFCTRL_STATE > FSM_STATE_RX_CALIBRATION);
    //         }
    //         res = cc2538_transmit_tx_buf();
    //         state = NETOPT_STATE_IDLE;
    //         res = RES_OK;
    //         break;

    //     case NETOPT_STATE_RESET:
    //         cc2538_off();
    //         cc2538_on();
    //         RFCORE_WAIT_UNTIL(RFCORE->XREG_FSMSTAT0bits.FSM_FFCTRL_STATE > FSM_STATE_RX_CALIBRATION);
    //         state = NETOPT_STATE_IDLE;
    //         res = RES_OK;
    //         break;
    // }

//     return res;
// }


static void isr(netdev2_t *dev)
{
    printf("[cc2538rf] netdev2: ISR called\n");

    // dev.event_callback(dev, NETDEV2_EVENT_ISR, NULL);
}

static int init(netdev2_t *dev)
{
    printf("[cc2538rf] netdev2: INIT called\n");


    cc2538rf_init();
    return 0;
}

static int send(netdev2_t *dev, const struct iovec *vector, int count)
{
    printf("[cc2538rf] netdev2: SEND called\n");

    // int len = 0;

    // lock(dev);

    // radio_tx_status_t result;

    // typeof(count)i;
    // for (i = 0; i < count; i++) {

    //     /* total frame size */
    //     if (CC2538_PACKET_LENGTH_SIZE + vector[i].iov_len > CC2538_RF_FIFO_SIZE) {
    //         return RADIO_TX_PACKET_TOO_LONG;
    //     }

    //     if (RFCORE->XREG_FSMSTAT1bits.TX_ACTIVE) {
    //         return RADIO_TX_MEDIUM_BUSY;
    //     }

    //     /* sequence number */
    //     hdr.sequence_nr = sequence_nr++;

    //     RFCORE_ASSERT(cc2538_is_on());

    //     rfcore_strobe(ISFLUSHTX);

    //     /* Send the phy length byte first */
    //     rfcore_write_byte(vector[i].iov_len + CC2538_AUTOCRC_LEN);
    //     rfcore_write_fifo(&hdr, hdr_size);
    //     rfcore_write_fifo(vector[i].iov_base, vector[i].iov_len);
    //     rfcore_tx_frame_hook(buf, len);

    //     result = cc2538_transmit_tx_buf();

    //     bool was_off = false;

    //     if (!cc2538_is_on()) {
    //         was_off = true;
    //         cc2538_on();
    //         RFCORE_WAIT_UNTIL(RFCORE->XREG_FSMSTAT0bits.FSM_FFCTRL_STATE > FSM_STATE_RX_CALIBRATION);
    //     }

    //     if (!cc2538_channel_clear()) {
    //         return RADIO_TX_COLLISION;
    //     }

    //     if (RFCORE->XREG_FSMSTAT1bits.SFD) {
    //         return RADIO_TX_COLLISION;
    //     }

    //     /* Start the transmission */
    //     rfcore_strobe(ISTXON);

    //     if (was_off) {
    //         /* Wait for transmission to start */
    //         RFCORE_WAIT_UNTIL(RFCORE->XREG_FSMSTAT1bits.TX_ACTIVE);

    //         cc2538_off();
    //     }

    //     if (result == RADIO_TX_OK) {
    //         len += vector[i].iov_len;
    //     }
    // }

    // unlock(dev);

    return count;
}

static int recv(netdev2_t *dev, char *buf, int len, void *info)
{
    printf("[cc2538rf] netdev2: RCV called\n");

    // uint_fast8_t rx_len;
    int result = 0;

    // if (RFCORE_XREG_RXFIFOCNT < 1) {
    //     DEBUG("%s(): RXFIFOCNT = 0!\n", __FUNCTION__);
    // }
    // else {
    //     rx_len = rfcore_read_byte();

    //     if (rx_len < CC2538_FCS_LEN) {
    //         DEBUG("%s(): length %u < CC2538_FCS_LEN!\n", __FUNCTION__, rx_len);
    //     }
    //     else if (rx_len > RFCORE_XREG_RXFIFOCNT) {
    //         DEBUG("%s(): length %u > RXFIFOCNT!\n", __FUNCTION__, rx_len);
    //         RFCORE_FLUSH_RECEIVE_FIFO();
    //     }
    //     else {
    //         assert(rx_len <= len); /* TODO */
    //         rfcore_read_fifo(buf, rx_len);
    //         result = rx_len;
    //     }
    // }

    return result;
}

static int get(netdev2_t *dev, netopt_t opt, void *value, size_t max_len)
{
    int res = 0;
    uint16_t tmp;

    switch (opt) {

        case NETOPT_CHANNEL:
            tmp = cc2538rf_get_channel();
            memcpy(value, &tmp, 2);
            return 2;

        // case NETOPT_DEVICE_TYPE:
        //     write_int(value, max_len, NETDEV2_TYPE_802154);
        //     res = max_len;
        //     break;

        // case NETOPT_ADDRESS:
        //     if (max_len >= CC2538_SHORT_ADDR_LEN) {
        //         uint8_t addr[CC2538_SHORT_ADDR_LEN];
        //         write_be(addr, CC2538_SHORT_ADDR_LEN, cc2538_get_address_long());
        //         ieee802154_get_iid(value, addr, CC2538_SHORT_ADDR_LEN);
        //         res = CC2538_SHORT_ADDR_LEN;
        //     }
        //     else {
        //         res = -EOVERFLOW;
        //     }
        //     break;

        // case NETOPT_ADDRESS_LONG:
        //     if (max_len >= CC2538_LONG_ADDR_LEN) {
        //         uint8_t addr[CC2538_LONG_ADDR_LEN];
        //         write_be(addr, CC2538_LONG_ADDR_LEN, cc2538_get_address_long());
        //         ieee802154_get_iid(value, addr, CC2538_LONG_ADDR_LEN);
        //         res = CC2538_LONG_ADDR_LEN;
        //     }
        //     else {
        //         res = -EOVERFLOW;
        //     }
        //     break;

        // case NETOPT_ADDR_LEN:
        // case NETOPT_SRC_LEN:
        //     write_int(value, max_len, use_long_addr ? CC2538_LONG_ADDR_LEN : CC2538_SHORT_ADDR_LEN);
        //     res = max_len;
        //     break;

        // case NETOPT_PROMISCUOUSMODE:
        //     write_int(value, max_len, cc2538_get_monitor());
        //     res = max_len;
        //     break;
        // case NETOPT_IPV6_IID:
        //     if (addr_is_long) {
        //         if (max_len >= CC2538_LONG_ADDR_LEN) {
        //             uint8_t addr[CC2538_LONG_ADDR_LEN];
        //             write_be(addr, CC2538_LONG_ADDR_LEN, cc2538_get_address_long());
        //             ieee802154_get_iid(value, addr, CC2538_LONG_ADDR_LEN);
        //             res = CC2538_LONG_ADDR_LEN;
        //         }
        //         else {
        //             res = -EOVERFLOW;
        //         }
        //     }
        //     else {
        //         if (max_len >= CC2538_SHORT_ADDR_LEN) {
        //             uint8_t addr[CC2538_SHORT_ADDR_LEN];
        //             write_be(addr, CC2538_SHORT_ADDR_LEN, cc2538_get_address_long());
        //             ieee802154_get_iid(value, addr, CC2538_SHORT_ADDR_LEN);
        //             res = CC2538_SHORT_ADDR_LEN;
        //         }
        //         else {
        //             res = -EOVERFLOW;
        //         }
        //     }
        //     break;

        // case NETOPT_IS_WIRED:
        //     res = -ENOTSUP;
        //     break;


        // case NETOPT_IS_CHANNEL_CLR:      /**< check if channel is clear */
        //     ((uint8_t *)value)[0] = cc2538_channel_clear() ? 1 : 0;
        //     for (n = 1; n < max_len; n++) ((uint8_t *)value)[n] = 0;
        //     res = 1;
        //     break;

        // case /**
        //         case  * @brief    get/set the network ID as uint16_t in host byte order
        //         case  *
        //         case  * Examples for this include the PAN ID in IEEE 802.15.4
        //         case  */
        // case NETOPT_NID:

        // case NETOPT_TX_POWER:
        //     write_int(value, max_len, cc2538_get_tx_power());
        //     res = max_len;
        //     break;

        // case NETOPT_MAX_PACKET_SIZE:
        //     write_int(value, max_len, CC2538_RF_MAX_PACKET_LEN);
        //     res = max_len;
        //     break;

        // case NETOPT_PRELOADING:
        //     write_int(value, max_len, use_preloading);
        //     res = max_len;
        //     break;

        // case NETOPT_AUTOACK:
        //     write_int(value, max_len, RFCORE->XREG_FRMCTRL0bits.AUTOACK);
        //     res = max_len;
        //     break;

        // case NETOPT_PROTO:
        //     write_int(value, max_len, GNRC_NETTYPE_UNDEF);
        //     res = max_len;
        //     break;

        // case NETOPT_STATE:
        //     write_int(value, max_len, state);
        //     res = max_len;
        //     break;

        // case NETOPT_RAWMODE:
        //     *(netopt_enable_t *)val = 1;
        //     res = 1;
        //     break;

        // case NETOPT_AUTOCCA:
        // case NETOPT_CSMA:
        //     *(netopt_enable_t *)val = (txon_instr == ISTXONCCA) ? 1 : 0;
        //     res = 1;
        //     break;

        // case NETOPT_CSMA_RETRIES:            /**< get/set the number of retries when when channel is busy */
        //     write_int(value, max_len, 0);
        //     res = max_len;
        //     break;

        default:
            res = -ENOTSUP;
            break;
    }

    return res;
}

static int set(netdev2_t *dev, netopt_t opt, void *value, size_t value_len)
{
    int res = 0;

    printf("[cc2538rf] netdev2: SET called with opt [%i]\n", (int)opt);

    switch (opt) {
        // case NETOPT_ADDR_LEN:
        // case NETOPT_SRC_LEN:
        //     switch (read_int(value, value_len)) {
        //         case CC2538_SHORT_ADDR_LEN:
        //             use_long_addr = false;
        //             res = value_len;
        //             break;

        //         case CC2538_LONG_ADDR_LEN:
        //             use_long_addr = true;
        //             res = value_len;
        //             break;

        //         default:
        //             res = -ENOTSUP;
        //             break;
        //     }
        //     break;

        // case NETOPT_CHANNEL:
        //     cc2538_set_channel(read_int(value, value_len));
        //     res = value_len;
        //     break;

        // case NETOPT_IS_CHANNEL_CLR:      /**< check if channel is clear */
        //     ((uint8_t *)value)[0] = cc2538_channel_clear() ? 1 : 0;
        //     for (n = 1; n < value_len; n++) ((uint8_t *)value)[n] = 0;
        //     res = 1;
        //     break;

        // case NETOPT_ADDRESS:             /**< get/set address in host byte order */
        //     cc2538_set_address(read_int(value, value_len));
        //     addr_is_long = false;
        //     res = value_len;
        //     break;

        // case NETOPT_ADDRESS_LONG:
        //     cc2538_set_address_long(read_int(value, value_len));
        //     addr_is_long = true;
        //     res = value_len;
        //     break;

        // case *
        //         case  * @brief    get/set the network ID as uint16_t in host byte order
        //         case  *
        //         case  * Examples for this include the PAN ID in IEEE 802.15.4
        //         case
        // case NETOPT_NID:

        // case NETOPT_TX_POWER:
        //     cc2538_set_tx_power(read_int(value, value_len));
        //     res = value_len;
        //     break;

        // case NETOPT_PRELOADING:
        //     use_preloading = (read_int(value, value_len) != 0);
        //     res = value_len;
        //     break;

        // case NETOPT_PROMISCUOUSMODE:
        //     cc2538_set_monitor(read_int(value, len));
        //     res = 1;
        //     break;

        // case NETOPT_AUTOACK:
        //     RFCORE->XREG_FRMCTRL0bits.AUTOACK = read_int(value, value_len);
        //     res = value_len;
        //     break;

        // case NETOPT_PROTO:
        //     res = (read_int(value, value_len) == GNRC_NETTYPE_UNDEF) ? value_len : -ENOTSUP;
        //     break;

        // case NETOPT_STATE:
        //     res = set_state(read_int(value, value_len));
        //     break;

        // case NETOPT_RAWMODE:
        //     res = -ENOTSUP;
        //     break;

        // case NETOPT_AUTOCCA:
        // case NETOPT_CSMA:
        //     txon_instr = (int_value == 0) ? ISTXON : ISTXONCCA;
        //     res = 1;
        //     break;

        // case NETOPT_CSMA_RETRIES:            /**< get/set the number of retries when when channel is busy */
        //     res = (read_int(value, value_len) == 0) ? value_len : -ENOTSUP;
        //     break;

        default:
            res = -ENOTSUP;
            break;
    }

    return res;
}


const netdev2_driver_t cc2538rf_driver = {
    .send   = send,
    .recv   = recv,
    .init   = init,
    .isr    = isr,
    .get    = get,
    .set    = set,
};
