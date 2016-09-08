

extern uint32_t _sinitlist;
extern uint32_t _einitlist;
extern uint32_t _speriphparams;
extern uint32_t _eperiphparams;
extern uint32_t _sdevparams;
extern uint32_t _edevparams;
extern uint32_t _sthreadparams;
extern uint32_t _ethreadparams;


void auto_init(void)
{
    for (void (*initfunc)(void) = &_sinitlist; module < &einitlist; ) {
        initfunc();
        ++initfunc;
    }

    /* TODO: initialize shared MCU peripherals, e.g. SPI, I2C */

    /* count number of defined devices and add placeholders */
    unsigned num = DEV_CONF_NUM_SLOTS;
    dev_param_t *p = (dev_param_t *)&_sdevparams
    while (p < &_edevparams) {
        ++num;
        p += p.psize / sizeof(void *);       /* TODO: alignment issues?! */
    }

    /* allocate memory for device list */
    dev_list_start = kinit_malloc(num * sizeof(rootdev *));
    dev_list_len = num;

    num = 0;
    dev_param_t *p = (dev_param_t *)&_sdevparams
    while (p < &_edevparams) {
        p.core->setup(NULL, p, num++);
        p += p.psize / sizeof(void *);       /* TODO: alignment issues?! */
    }

    /* bootstrap predefined threads */
    thread_params_t *tp = (thread_params_t *)&_sthreadparams;
    while (tp < &_ethreadparams) {
        void *mem = kinit_malloc(tp->stacksize);
        thread_init(mem, tp->stacksize, tp->prio, tp->flags,
                    tp->func, tp->arg, tp->name);
        ++tp;
    }
}
