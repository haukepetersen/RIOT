/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_facemaster_faceimp_154
 *
 * # TODO
 * - many many things...
 *
 *
 * @{
 *
 * @file
 * @brief       Implementation of netdev-based IEEE802.15.4 based faceimp
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "log.h"
#include "assert.h"

#include "facemaster.h"
#include "faceimp/154.h"

#define SYNC_MSG            (0xaffe)
#define SEQ_NUM_INIT        (0x23)      // TODO: do we need to randomize this?

#define MAX_PDU_LEN         (IEEE802154_FRAME_LEN_MAX - 16) // TODO:

static char _stack[FACEIMP_154_STACKSIZE];
static msg_t _msgq[FACEIMP_154_MSGQSIZE];
static kernel_pid_t _handler_pid;

static int _send(faceimp_t *imp, void *data, size_t len)
{
    assert(imp);
    assert(data && (len > 0));

    faceimp_154_dev_t *impdev = imp->impdev;

    mutex_lock(&impdev->txlock);

    if (len >= MAX_PDU_LEN) {
        return FACEMASTER_NOBUF;
    }

    // TODO: assemble 802.15.4 header and move data into the same buffer
    int pkt_len = ieee802154_set_frame_hdr(impdev->txbuf,
                                           impdev->addr,
                                           FACEIMP_154_ADDRLEN,
                                           imp->addr,
                                           FACEIMP_154_ADDRLEN,
                                           0, impdev->cfg->pan_id,
                                           IEEE802154_FCF_PAN_COMP,
                                           impdev->seq++);
    if (pkt_len == 0) {
        return FACEMASTER_FACEERR;
    }
    memcpy(&imp->txbuf[pkt_len], data, len);
    pkt_len += len;

    // TODO: check return value of send function
    int res = impdev->dev.driver->send(&impdev->dev, impdev->txbuf, (size_t)pkt_len);

    mutex_unlock(&impdev->txlock);

    if (res != pkt_len) {
        return FACEMASTER_FACEERR;
    }
    return FACEMASTER_OK;
}

static void _recv(netdev_t *dev)
{
    faceimp_154_dev_t *impdev = (faceimp_154_dev_t *)dev->context;

    size_t pkt_len = impdev->dev.driver->recv(dev, impdev->rxbuf,
                                              IEEE802154_FRAME_LEN_MAX, NULL);

    // TODO: read src address
    // TODO: map src address to Imp -> seems we need a list of connected Imps?!
    // TODO: trigger receive facemaster's receive function passing Imp
}

static void *_handler_thread(void *arg)
{
    (void)arg;
    msg_t msg;

    /* golfer or not a golfer... */
    _handler_pid = thread_getpid();
    /* we need a message queue here */
    msg_init_queue(_msgq, FACEIMP_154_MSGQSIZE);

    while (1) {
        msg_receive(&msg);
        if (msg->type == SYNC_MSG) {
            netdev_t *dev = (netdev_t *)msg.content.ptr;
            dev->driver->isr(dev);
        }
        else {
            LOG_WARNING("[faceimp_154] thread received unknown message type\n");
        }
    }

    /* never reached */
    return NULL;
}

static void _netdev_eventhandler(netdev_t *dev, netdev_event_t event)
{
    switch (event) {
        case NETDEV_EVENT_ISR: {
            msg_t msg = { .type = SYNC_MSG, .content.ptr = dev };
            int res = msg_send_int(&msg, _handler_pid);
            if (res != 1) {
                LOG_WARNING("[faceimp_154] interrupt lost!\n");
            }
            break;
        }
        case NETDEV_EVENT_RX_COMPLETE:
            _on_data(dev);
            break;
        default:
            /* nothing to be done in this case... */
            break;
    }
}

int faceimp_154_init(void)
{
    /* run handler thread */
    thread_create(_stack, sizeof(_stack),
                  FACEIMP_154_THREAD_PRIO,
                  THREAD_CREATE_STACKTEST,
                  _handler_thread, NULL,
                  "faceimp_154");

    return FACEMASTER_OK;
}

int faceimp_154_add_dev(faceimp_154_dev_t *impdev, netdev_t *netdev,
                        const faceimp_154_cfg_t *cfg)
{
    assert(impdev);
    assert(netdev && netdev->driver);
    assert(cfg);

    /* make sure the given device is actually a 802.15.4 device */
    uint16_t devtype = 0;
    int res = netdev->driver->get(netdev, NETOPT_DEVICE_TYPE, &devtype, 2);
    if ((res != 2) || (devtype != NETDEV_TYPE_IEEE802154)) {
        return FACEMASTER_DEVERR;
    }

    /* setup device context */
    mutex_init(&impdev->txlock);
    impdev->seq_num = SEQ_NUM_INIT;

    /* setup actual device driver (netdev) */
    netdev->event_callback = _netdev_eventhandler;
    netdev->context = impdev;

    /* do the actual network device initialization */
    res = netdev->driver->init(netdev);
    if (res != 0) {
        return FACEMASTER_FACEERR;
    }

    /* once the device is running, its time to configure it */
    res = netdev->driver->set(netdev, NETOPT_CHANNEL, impdev->cfg->chan, 2);
    if (res != 2) {
        return FACEMASTER_DEVERR;
    }
    res = netdev->driver->set(netdev, NETOPT_NID, impdev->cfg->pan_id, 2);
    if (res != 2) {
        return FACEMASTER_DEVERR;
    }
#if FACEIMP_154_ADDRLEN == IEEE802154_SHORT_ADDRESS_LEN
    res = netdev->driver->get(netdev, NETOPT_ADDRESS, impdev->addr, 2);
#else
    res = netdev->driver->get(netdev, NETOPT_ADDRESS_LONG, impdev->addr, 8);
#endif
    if (res != 2) {
        return FACEMASTER_DEVERR;
    }

    /* once the network device is good, we setup and register the device
     * specific broadcast face */
    impdev->bcast_face.impdev = impdev;
    memcpy(impdev->bcast_face.addr, IEEE802154_ADDR_BCAST, IEEE802154_SHORT_ADDRESS_LEN);
    faceimp_init(&impdev->bcast_face.imp);
    facemaster_up(&impdev->bcast_face); // TODO: how to signal this is a bcast face?!

    return FACEMASTER_OK
}

/* to be called from: user application or faceman module */
int faceimp_154_connect(faceimp_154_dev_t *impdev,
                        faceimp_154_t *imp,
                        uint16_t addr)
{
    assert(impdev);
    assert(imp);

    /* lets make sure there is no duplicate entry for the bcast address */
    if (memcmp(addr, IEEE802154_ADDR_BCAST, IEEE802154_SHORT_ADDRESS_LEN) == 0) {
        return FACEMASTER_NOADDR;
    }

    imp->impdev = impdev;
    memcpy(imp->addr, addr, IEEE802154_SHORT_ADDRESS_LEN); // TODO: byteorder conversion?!
    faceimp_init(&imp->imp);

    int res = facemaster_up(imp);
    if (res != FACEMASTER_OK) {
        return res;
    }

    // TODO: call user supplied event notification function...

    return FACEMASTER_OK;
}

int faceimp_154_terminate(faceimp_154_t *imp)
{
    assert(imp);

    int res = facemaster_down(imp);
    if (res != FACEMASTER_OK) {
        return res;
    }

    // TODO: call user supplied event notification function...

    return FACEMASTER_OK;
}
