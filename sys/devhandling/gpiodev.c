

typedef struct {
    dev_params_t base;
    gpio_t pin;
    gpio_mode_t mode;
} saul_gpio_params_t;

typedef struct {
    saul_t saul;
    gpio_t pin;
} saul_gpio_t;


static void setup(void *mem, dev_params_t *params, int pos)
{
    saul_gpio_t *dev = saul_basesetup(mem, params, sizeof(saul_gpio_t), pos,
                                      &saul_gpio_driver);
    dev->pin = ((saul_gpio_params_t *)params)->pin;
}

static void init(dev_t *root)
{
    saul_gpio_params_t *p = (saul_gpio_params_t)root.p;
    gpio_init(p.pin, p.mode);
}

static int read(saul_t *saul, phydat_t *res)
{
    res->val[0] = (gpio_read(((saul_gpio_t *)saul)->pin)) ? 1 : 0;
    memset(&(res->val[1]), 0, 2 * sizeof(int16_t));
    res->unit = UNIT_BOOL;
    res->scale = 0;
    return 1;
}

static int write(saul_t *saul, phydat_t *state)
{
    gpio_write(((saul_gpio_t *)saul)->pin, state->val[0]);
    return 1;
}

const saul_driver_t gpio_saul_driver = {
    .read       = read,
    .write      = write,
};

const dev_core_t saul_gpio_core = {
    .type.full  = DEV_PERIPH_GPIO_GENERIC,
    .psize      = sizeof(saul_gpio_params_t),
    .setup      = setup,
    .init       = init,
    .get        = dev_notsup,
    .set        = dev_notsup
};
