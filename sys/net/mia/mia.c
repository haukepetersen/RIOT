
#include <errno.h>

#include "net/ethernet.h"

#include "net/mia.h"
#include "net/mia/eth.h"
#include "net/mia/udp.h"

#ifdef MODULE_MIA_DHCP
    #include "net/mia/dhcp.h"
#endif
#ifdef MODULE_MIA_COAP
    #include "net/mia/coap.h"
#endif


#define ENABLE_DEBUG            (0)
#include "debug.h"




netdev2_t *mia_dev;
mutex_t mia_mutex;
uint8_t mia_buf[MIA_BUFSIZE];

const uint8_t mia_bcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static msg_t msg_queue[MIA_MSG_QUEUESIZE];
static kernel_pid_t mia_pid;

static void on_evt(netdev2_t *dev, netdev2_event_t evt)
{
    msg_t msg;

    switch (evt) {
        case NETDEV2_EVENT_ISR:
            msg.type = MIA_MSG_ISR;
            msg_send(&msg, mia_pid);
            break;
        case NETDEV2_EVENT_RX_COMPLETE:
            mia_lock();
            dev->driver->recv(dev, (char *)mia_buf, MIA_BUFSIZE, NULL);
            mia_eth_process();
            mia_unlock();
            break;
        default:
            /* not interested in anything else for now */
            break;
    }
}

int mia_run(netdev2_t *netdev)
{
    uint16_t type;
    msg_t msg;
    mia_dev = netdev;

    DEBUG("[mia] starting stack initialization\n");

    /* device must be given */
    if (mia_dev == NULL) {
        return -ENODEV;
    }
    /* so far we can only handle Ethernet */
    mia_dev->driver->get(mia_dev, NETOPT_DEVICE_TYPE, (void *)&type, 2);
    if (type != NETDEV2_TYPE_ETHERNET) {
        return -ENOTSUP;
    }

    /* init mutex */
    mutex_init(&mia_mutex);
    /* setup the message queue and remember the PID */
    msg_init_queue(msg_queue, MIA_MSG_QUEUESIZE);
    mia_pid = thread_getpid();
    /* bootstrap the network device */
    mia_dev->driver->init(mia_dev);
    mia_dev->event_callback = on_evt;
    mia_dev->context = NULL;

    /* bind build-in endpoints */
#ifdef MODULE_MIA_DHCP
    mia_udp_bind(&mia_dhcp_ep);
#endif
#ifdef MODULE_MIA_COAP
    mia_udp_bind(&mia_coap_ep);
#endif

    /* get the MAC address from the device driver */
    mia_dev->driver->get(mia_dev, NETOPT_ADDRESS, &mia_mac, ETHERNET_ADDR_LEN);

    DEBUG("[mia] initialization seems ok, running now\n");
    while (1) {
        msg_receive(&msg);
        if (msg.type == MIA_MSG_ISR) {
            mia_dev->driver->isr(mia_dev);
        }
    }

    /* we should not end up here... */
    return 0;
}
