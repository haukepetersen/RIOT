/*
 * Copyright (C) 2018 Freie Universität Berlin
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
 * @brief       Helper tool for measuring BLE connection events
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "board.h"
#include "sched.h"
#include "thread_flags.h"

#include "net/gorm/ll.h"
#include "nrfble_rxtim.h"
#include "net/netdev/ble.h"


#define ADV_AA              (0x8e89bed6)
#define ADV_CRC             (0x00555555)
#define ADV_CHAN            (39U)

#define SYNC_ADV            (0x0001)
#define SYNC_DAT            (0x0002)

#define HOP_MASK(x)         (x & 0x1f)

typedef struct __attribute__((packed)) {
    uint8_t inita[GORM_LL_ADDR_LEN];
    uint8_t adva[GORM_LL_ADDR_LEN];
    uint8_t aa[4];
    uint8_t crc[GORM_LL_CRC_LEN];
    uint8_t win_size;
    uint8_t win_offset[2];
    uint8_t interval[2];
    uint8_t latency[2];
    uint8_t timeout[2];
    uint8_t chan_map[GORM_LL_CHANMAP_LEN];
    uint8_t hop_sca;
} connect_ind_mtu_t;

/* state config */
static netdev_ble_pkt_t pkt;
static netdev_t *dev;
static thread_t *thread;
static thread_flags_t flag = SYNC_ADV;

static gorm_ll_connection_t con;

/* timings */
static uint16_t evt_cnt = 0xffff;
static uint32_t last = 0;
static uint32_t rxtim = 0;


static int _chan_used(uint8_t *map, uint8_t num)
{
    return (map[num >> 3] & (1 << (num & 0x07)));
}

uint8_t gorm_ll_chan_count(uint8_t *map)
{
    uint8_t cnt = 0;
    for (uint8_t i = 0; i < GORM_LL_CHAN_NUMOF; i++) {
        if (_chan_used(map, i)) {
            ++cnt;
        }
    }
    return cnt;
}

void gorm_ll_chan_algo1(gorm_ll_connection_t *con)
{
    con->chan_unmapped = (con->chan_unmapped + con->chan_hop) %
                         GORM_LL_CHAN_DATA_NUMOF;
    if (_chan_used(con->chan_map, con->chan_unmapped)) {
        con->ctx.chan = con->chan_unmapped;
    }
    else {
        unsigned remap_index = (con->chan_unmapped % con->chan_cnt) + 1;
        int pos = -1;
        while (remap_index > 0) {
            /* advance to the next used channel */
            while (_chan_used(con->chan_map, ++pos) == 0) {}
            --remap_index;
        }
        con->ctx.chan = pos;
    }
}



static inline uint16_t _letohs(uint8_t *buf)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (uint16_t)((buf[1] << 8) | buf[0]);
#else
    return (uint16_t)((buf[0] << 8) | buf[1]);
#endif
}


static void _on_radio(netdev_t *netdev, netdev_event_t event)
{
    (void)event;
    (void)netdev;

    LED0_OFF;

    rxtim = xtimer_now_usec();
    thread_flags_set(thread, flag);
}

static void _prep_next_pkt(void)
{
    evt_cnt++;

    if ((con.chan_update.cnt != 0) &&
        (evt_cnt == con.chan_update.instant)) {
        memcpy(con.chan_map, con.chan_update.map, 5);
        con.chan_cnt = con.chan_update.cnt;
        con.chan_update.cnt = 0;
        printf("--- chan update applied ---\n");
    }
    gorm_ll_chan_algo1(&con);
}

static void _check_for_tim_update(void)
{
    if (((pkt.flags & GORM_LL_LLID_MASK) == GORM_LL_LLID_CTRL)
         && (pkt.pdu[0] == GORM_LL_CONN_UPDATE_IND)) {
        uint8_t *d = &pkt.pdu[1];
        printf("--- connection update ---\n");
        uint16_t win_size = d[0];
        uint16_t win_offset = _letohs(&d[1]);
        uint16_t interval = _letohs(&d[3]);
        uint16_t timeout = _letohs(&d[7]);
        uint16_t instant = _letohs(&d[9]);
        printf("win_size:   %u\n", (unsigned)win_size);
        printf("win_offset: %u\n", (unsigned)win_offset);
        printf("interval:   %u\n", (unsigned)interval);
        printf("timeout:    %u\n", (unsigned)timeout);
        printf("instant:    %u\n", (unsigned)instant);
    }
}

static void _check_for_chan_update(void)
{
    if (((pkt.flags & GORM_LL_LLID_MASK) == GORM_LL_LLID_CTRL)
         && (pkt.pdu[0] == GORM_LL_CHANNEL_MAP_IND)) {
        uint8_t *d = &pkt.pdu[1];

        printf("--- channel map update ---\n");
        printf("map: %02x %02x %02x %02x %02x\n",
               (int)d[0], (int)d[1], (int)d[2], (int)d[3], (int)d[4]);
        uint16_t instant = _letohs(&d[5]);
        printf("instant: %i\n", (int)instant);

        memcpy(con.chan_update.map, d, 5);
        con.chan_update.instant = instant;
        con.chan_update.cnt = gorm_ll_chan_count(d);
        printf("count: %i\n", (int)con.chan_update.cnt);
    }
}

static void _check_for_con(void)
{
    if ((pkt.flags & GORM_LL_PDU_MASK) == GORM_LL_CONNECT_IND) {
        connect_ind_mtu_t *pdu = (connect_ind_mtu_t *)pkt.pdu;

        con.chan_hop = HOP_MASK(pdu->hop_sca);
        memcpy(con.chan_map, pdu->chan_map, GORM_LL_CHANMAP_LEN);
        con.chan_cnt = gorm_ll_chan_count(con.chan_map);
        con.chan_unmapped = 0;

        memcpy(con.ctx.aa.raw, pdu->aa, 4);
        memcpy(&con.ctx.crc, pdu->crc, GORM_LL_CRC_LEN);
        _prep_next_pkt();

        flag = SYNC_DAT;

        printf("New Connection\n");
        // printf("hop_inc:    %i\n", (int)con.chan_hop);
        // printf("aa:         0x%04x\n", (int)con.ctx.aa.u32);
        // printf("crc:        0x%03x\n", (int)con.ctx.crc);
        // printf("--- timings ---\n");
        // uint16_t win_size = pdu->win_size;
        // uint16_t win_offset = _letohs(pdu->win_offset);
        // uint16_t interval = _letohs(pdu->interval);
        // uint16_t timeout = _letohs(pdu->timeout);
        // printf("win_size:   %u\n", (unsigned)win_size);
        // printf("win_offset: %u\n", (unsigned)win_offset);
        // printf("interval:   %u\n", (unsigned)interval);
        // printf("timeout:    %u\n", (unsigned)timeout);
    }
}

int main(void)
{
    puts("BLE RX timer");

    /* setup environment */
    thread = (thread_t *)sched_active_thread;

    last = xtimer_now_usec();

    /* init radio */
    dev = nrfble_setup();
    dev->event_callback = _on_radio;
    dev->driver->init(dev);

    /* prepare packet */
    con.ctx.crc = ADV_CRC;
    con.ctx.aa.u32 = ADV_AA;
    con.ctx.chan = ADV_CHAN;

    while (1) {
        LED0_ON;
        netdev_ble_recv(dev, &pkt, &con.ctx);
        thread_flags_t reason = thread_flags_wait_any(SYNC_ADV | SYNC_DAT);

        if (reason & SYNC_ADV) {
            // printf("adv: %u [%u us] | ", (unsigned)rxtim, (unsigned)(last - rxtim));
            // _adv_info();
            _check_for_con();
        }
        if (reason & SYNC_DAT) {
            LED1_OFF;
            printf("evt %i: %u [%u us] - ch %i\n",
                   (int)evt_cnt, (unsigned)rxtim,
                   (unsigned)(rxtim - last),
                   (int)con.ctx.chan);
            _check_for_tim_update();
            _check_for_chan_update();
            _prep_next_pkt();
            LED1_ON;
        }

        last = rxtim;
    }

    return 0;
}
