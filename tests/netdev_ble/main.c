/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Low level test application for netdev based BLE radio drivers
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "shell_commands.h"
#include "shell.h"
#include "net/netdev/ble.h"

#ifdef MODULE_NRFBLE
#include "nrfble.h"
#else
#error "Error: no BLE capable netdev device found"
#endif

#define LL_ADV_AA           0x8e89bed6

static netdev_t *dev;
static netdev_ble_pkt_t rx_pkt;
static netdev_ble_pkt_t tx_pkt;
static volatile int in_rx = 0;

static void dump_pkt(netdev_ble_pkt_t *pkt)
{
    int type = (pkt->hdr.flags & NETDEV_BLE_HDR_PDU_MSK);
    int chsel = !(!(pkt->hdr.flags & NETDEV_BLE_HDR_CHSEL));
    int txadd = !(!(pkt->hdr.flags & NETDEV_BLE_HDR_TXADD));
    int rxadd = !(!(pkt->hdr.flags & NETDEV_BLE_HDR_RXADD));

    puts("~~~ BLE packet ~~~");
    printf("chan: %i\n", (int)pkt->chan);
    printf("  aa: 0x%08x\n", (int)pkt->aa.u32);
    printf(" hdr: PDU[0x%x] ChSel[%i] TxAdd[%i] RxAdd[%i]\n",
           type, chsel, txadd, rxadd);
    printf(" len: %i bytes\n", (int)pkt->hdr.len);
    printf("data: ");
    for (uint8_t i = 0; i < pkt->hdr.len; i++) {
        printf("%c", (char)pkt->pdu[i]);
    }
    printf("\n crc: 0x");
    for (int i = 0; i < 3; i++) {
        printf("%0x", (int)pkt->crc[i]);
    }
    puts("\n~~~ END ~~~");
}

static void on_netdev_evt(netdev_t *netdev, netdev_event_t event)
{
    (void)dev;
    assert(netdev == dev);

    printf("netdev event: %i\n", (int)event);
    switch (event) {
        case NETDEV_EVENT_RX_COMPLETE:
            dump_pkt(&rx_pkt);
            in_rx = 0;
            break;
        case NETDEV_EVENT_CRC_ERROR:
            puts("got packet, but CRC was wrong");
            in_rx = 0;
            break;
        default:
            puts("error: unknown netdev event");
    }
}

static int parse_context(netdev_ble_pkt_t *pkt, char **argv, int len)
{
    /* set default values */
    memset(pkt, 0, sizeof(netdev_ble_pkt_t));
    pkt->aa.u32 = LL_ADV_AA;
    pkt->chan = 39;
    pkt->hdr.flags = 0x42;
    memset(pkt->crc, 0x55, 3);

    if (len > 0) {
        pkt->chan = (uint8_t)atoi(argv[0]);
    }
    /* TODO: parse the rest, needs string hex to binary conversion */
    return 0;
}

static int cmd_tx(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <data> [chan [aa [crc]]]\n", argv[0]);
        return 1;
    }

    if (in_rx) {
        puts("error: unable to transmit data, receive operation in progress");
        return 1;
    }

    if (parse_context(&tx_pkt, &argv[2], argc - 2) != 0) {
        puts("error: unable to parse context");
        return 1;
    }

    size_t len = strlen(argv[1]);
    if (len > NETDEV_BLE_PDU_MAXLEN) {
        puts("error: payload exceeds packet size limit}");
        return 1;
    }
    memcpy(tx_pkt.pdu, argv[1], len);
    tx_pkt.hdr.len = (uint8_t)len;

    netdev_ble_send(dev, &tx_pkt);
    puts("Send packet:");
    dump_pkt(&tx_pkt);
    puts("done here.");

    return 0;
}

static int cmd_ibeacon(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (in_rx) {
        puts("error: unable to transmit data, receive operation in progress");
        return 1;
    }

    uint8_t pos = 0;
    uint8_t addr[6] = { 0x56, 0x34, 0x12, 0xef, 0xcd, 0xab };
    uint8_t prefix[9] = { 0x02, 0x01, 0x06, 0x1a, 0xff, 0x4c, 0x00, 0x02, 0x15 };
    uint32_t uuid[4] = { 0xa1b2c3d4, 0xaffecafe, 0xe3e1beef, 0x12345678 };
    uint16_t maj = 1;
    uint16_t min = 23;

    /* TMP HACK HACK HACK */
    parse_context(&tx_pkt, NULL, 0);
    memcpy(tx_pkt.pdu, addr, 6);
    pos += 6;
    memcpy(&tx_pkt.pdu[pos], prefix, 9);
    pos += 9;
    memcpy(&tx_pkt.pdu[pos], uuid, 16);
    pos += 16;
    tx_pkt.pdu[pos + 1] = (uint8_t)maj;
    tx_pkt.pdu[pos + 3] = (uint8_t)min;
    pos += 4;
    tx_pkt.pdu[29] = 0;
    tx_pkt.hdr.len = (pos + 1);

    for (uint8_t chan = 37; chan <= 39; chan ++) {
        tx_pkt.chan = chan;
        netdev_ble_send(dev, &tx_pkt);
        printf("send iBacon on channel %i\n", (int)chan);
    }

    return 0;
}

static int cmd_rx(int argc, char **argv)
{
    if (argc < 1) {
        printf("usage: %s [chan [aa [crc]]]\n", argv[0]);
        return 0;
    }

    if (in_rx) {
        puts("receive operation already in progress - doing nothing");
        return 1;
    }

    if (parse_context(&rx_pkt, &argv[1], argc - 1) != 0) {
        puts("error: unable to parse context");
        return 1;
    }
    in_rx = 1;
    netdev_ble_recv(dev, &rx_pkt);
    printf("waiting for incoming data on channel %i\n", (int)rx_pkt.chan);

    return 0;
}

static int cmd_abort(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    netdev_ble_recv(dev, NULL);
    in_rx = 0;

    puts("receive operation aborted");
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "tx", "transmit a packet", cmd_tx },
    { "ibeacon", "send iBeacon packet on each ADV channel", cmd_ibeacon },
    { "rx", "start receiving packet", cmd_rx },
    { "abort", "abort receive operation", cmd_abort },
    { NULL, NULL, NULL }
};

int main(void)
{
    printf("Low-level BLE radio driver test\n");

    /* initialize the BLE device */
#ifdef MODULE_NRFBLE
    dev = nrfble_setup();
    if (dev == NULL) {
        puts("error: unable to initialize nrfble driver");
        return -1;
    }
#endif
    dev->event_callback = on_netdev_evt;
    if (dev->driver->init(dev) != 0) {
        puts("error: unable to initialize BLE device");
        return 1;
    }

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
