




typedef struct {
    dev_params_t base;
    spi_t spi;
    gpio_t cs;
    abcdev_mode_t mode;
} abcdev_params_t;


typedef struct {
    saul_t saul;
    int some_state;
    int some_counter;
} abcdev_t;

/**
 * @brief   Bootstrap the device driver
 *
 * 1. allocate device descriptor
 * 2.
 */
static void setup(void *mem, dev_params_t *params, unsigned pos)
{
    /* generic device setup */
    devabc_t *dev = saul_basesetup(mem, params, sizeof(devabc_t),
                                   pos, &devabc_saul_driver);
    /* do device specific field initialization */
    dev->some_state = 0;
    dev->some_counter = 0;
}

static void init(dev_t *root)
{
    /* bring the actual device online ... */
}

static int read(saul_t *saul, phydat_t *res)
{
    /* blabla bla */
    return 1;
}

static int write(saul_t *saul, phydat_t *res)
{
    /* blubb blubb blubb */
    return 1;
}

const saul_driver_t abcdev_saul = {
    .read       = read,
    .write      = write
};

const dev_core_t abcdev_core = {
    .type.full  = DEV_SENSE_TEMP_ABCDEV,
    .psize      = sizeof(abcdev_params_t),
    .setup      = setup,
    .init       = init,
    .get        = get,
    .set        = set
};
