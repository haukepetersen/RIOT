
#include <errno.h>

#include "thread_flags.h"
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

#define RXFLAG                  (0x0400)


netdev_t *mia_dev;
mutex_t mia_mutex;
uint8_t mia_buf[MIA_BUFSIZE];

const uint8_t mia_bcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static thread_t *_mia_thread;

static void _on_evt(netdev_t *dev, netdev_event_t evt)
{
    switch (evt) {
        case NETDEV_EVENT_ISR:
            thread_flags_set(_mia_thread, RXFLAG);
            break;
        case NETDEV_EVENT_RX_COMPLETE:
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

int mia_run(netdev_t *netdev)
{
    uint16_t type;
    mia_dev = netdev;
    _mia_thread = (thread_t *)thread_get(thread_getpid());

    DEBUG("[mia] starting stack initialization\n");
    /* init mutex */
    mutex_init(&mia_mutex);

    /* device must be given */
    if (mia_dev == NULL) {
        return -ENODEV;
    }
    /* so far we can only handle Ethernet */
    mia_dev->driver->get(mia_dev, NETOPT_DEVICE_TYPE, (void *)&type, 2);
    if (type != NETDEV_TYPE_ETHERNET) {
        return -ENOTSUP;
    }

    /* bootstrap the network device */
    mia_dev->driver->init(mia_dev);
    mia_dev->event_callback = _on_evt;
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
        thread_flags_wait_all(RXFLAG);
        mia_dev->driver->isr(mia_dev);
    }

    /* we should not end up here... */
    return 0;
}

int mia_init(void)
{


    return MIA_OK;
}
