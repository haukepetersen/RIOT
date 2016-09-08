




typedef struct {
    dev_params_t base;
    spi_t spi;
    spi_speed_t spi_speed;
    gpio_t cs_pin;
    gpio_t int_pin;
    gpio_t rst_pin;
} somenet_params_t;


typedef struct {
    netdev2_ieee802154_t netdev;
    int some_state;
    int some_counter;
} abcdev_t;

/**
 * @brief   Bootstrap the device driver
 *
 * 1. allocate device descriptor
 * 2.
 */
static void setup(void *params, int pos)
{
    /* allocate device descriptor */
    devabc_t *dev = (devabc_t *)kinit_malloc(sizeof(devabc_t));
    /* link to params and create shortcut to device API */
    dev->gen.p = (dev_params_t *)params;
    dev->api = (saul_driver_t *)dev->gen.p->root.api;
    /* do device specific field initialization */
    dev->some_state = 0;
    dev->some_counter = 0;
    /* finally, add device to device list */
    dev_list[pos] = dev;
}

const netdev2_driver_t somenet_api_netdev = {
    .send = send,
    .recv = recv,
    .isr  = isr,
};

const rdev_t somenet_core = {
    .type.full  = DEV_NET_ETH_SOMENET,
    .psize      = sizeof(somenet_params_t),
    .api        = &somenet_api_netdev,
    .setup      = setup,
    .init       = init,
    .get        = get,
    .set        = set
};
