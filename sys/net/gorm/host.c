
/* TODO: factor out thread flag stuff into something ARCH dependent */
#include "thread_flags.h"

#include "net/gorm/ll.h"
#include "net/gorm/ll/ctrl.h"
#include "net/gorm/l2cap.h"

#define ENABLE_DEBUG                (1)
#include "debug.h"

#define NOTIFY_FLAG         (0x0001)

static thread_t *_host_thread;

/* allocate memory for the defined number of possible connections */
static gorm_ctx_t _ctx[GORM_CFG_CONNECTIONS_LIMIT];

static void _on_data(gorm_ctx_t *con, gorm_buf_t *buf)
{
    uint8_t llid = (buf->pkt.flags & GORM_LL_LLID_MASK);

    switch (llid) {
        case GORM_LL_LLID_CTRL:
            gorm_ll_ctrl_on_data(con, buf);
            break;
        case GORM_LL_LLID_DATA_CONT:
        case GORM_LL_LLID_DATA_START:
            gorm_l2cap_on_data(con, llid, buf);
            break;
        default:
            DEBUG("[gorm_ll_host] on_data: invalid LLID value\n");
            gorm_buf_return(buf);
            break;
    }

    DEBUG("~~~~ pkt done, pool size is %u\n", gorm_buf_count(&gorm_buf_pool));
}

void gorm_host_run(void)
{
    _host_thread = (thread_t *)sched_active_thread;

    while (thread_flags_wait_any(NOTIFY_FLAG)) {
        DEBUG("[gorm_ll_host] run: got notified about incoming data! ~~~~~~~~~~~\n");

        /* TODO: find better and more efficient way to select and dispatch the
         *       the pending data! */
        /* idea: could we limit the max number of connections to 16 and use
         *       dedicated thread flags for each connection? -> dont like the limit*/
        gorm_ctx_t *list = gorm_ll_periph_getcon();
        for (unsigned i = 0; i < GORM_CFG_LL_PERIPH_CONNECTIONS_LIMIT; i++) {
            gorm_ctx_t *con = &list[i];
            gorm_buf_t *buf;
            while ((buf = gorm_buf_deq(&con->ll.rxq))) {
                _on_data(con, buf);
            }
        }
    }
}

void gorm_host_notify(gorm_ctx_t *con)
{
    (void)con;
    thread_flags_set(_host_thread, NOTIFY_FLAG);
}





void gorm_ll_host_init(void)
{
    /* TODO */
}


/* TODO: redo event passing to Host, so this will not be needed anymore */
gorm_ctx_t *gorm_ll_periph_getcon(void)
{
    return connections;
}

static uint8_t _get_state(gorm_ctx_t *con)
{
    return (con->ll.state & STATE_MASK);
}

/* TODO: remove this function once we move the connection memory handling up the
 *       stack */
static gorm_ctx_t *_get_by_state(uint8_t state)
{
    for (unsigned i = 0; i < GORM_CFG_LL_PERIPH_CONNECTIONS_LIMIT; i++) {
        if (_get_state(&connections[i]) == state) {
            return &connections[i];
        }
    }
    return NULL;
}




/* TODO: move to host */
void gorm_ll_periph_init(void)
{
    /* address(es) */
    /* TODO: refine and fix: move to GAP and integrate with ll_host */
    memcpy(adv_data.addr, gorm_ll_addr_rand(), BLE_ADDR_LEN);

    /* initialize connection memory */
    /* TODO: move up the stack to host -> YES */
    for (unsigned i = 0; i < GORM_CFG_LL_PERIPH_CONNECTIONS_LIMIT; i++) {
        memset(&connections[i], 0, sizeof(gorm_ctx_t));
        connections[i].ll.state = STATE_STANDBY;
    }
}

/* TODO: move to host */
void gorm_ll_periph_adv_setup(void *ad_adv, size_t adv_len,
                              void *ad_scan, size_t scan_len)
{
    adv_data.ad_adv = ad_adv;
    adv_data.ad_adv_len = adv_len;
    adv_data.ad_scan = ad_scan;
    adv_data.ad_scan_len = scan_len;
}

/* TODO: move to host */
void gorm_host_adv_start(void)
{
    DEBUG("[gorm_ll_periph] adv_start: trying to trigger advertising now\n");

    /* make sure we are not advertising at the moment */
    /* TODO: solve this using some global state variable/pointer? */
    if (_get_by_state(STATE_ADV) != NULL) {
        DEBUG("[gorm_ll_periph] adv_start: advertisement in progress...\n");
        return;
    }

    /* get any non-connected slot */
    /* TODO: use conn_interval for this, get rid of state field */
    gorm_ctx_t *con = _get_by_state(STATE_STANDBY);
    if (con == NULL) {
        DEBUG("[gorm_ll_periph] adv_start: not possible: all slots taken\n");
        return;
    }

    gorm_ll_advertise(&con->ll);
}

void gorm_ll_periph_terminate(gorm_ll_ctx_t *con)
{
    unsigned is = irq_disable();
    gorm_ll_trx_stop();
    gorm_arch_timer_cancel(&con->timer_con);
    gorm_arch_timer_cancel(&con->timer_spv);
    con->state = STATE_STANDBY;
    irq_restore(is);

    /* TODO:
     * - release buffers
     * - behave differently depending on the current state (ADV vs CONNECTED) */
}
