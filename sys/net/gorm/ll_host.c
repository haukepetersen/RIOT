
/* TODO: factor out thread flag stuff into something ARCH dependent */
#include "thread_flags.h"

#include "net/gorm/ll.h"
#include "net/gorm/ll/ctrl.h"
#include "net/gorm/l2cap.h"

#define ENABLE_DEBUG                (1)
#include "debug.h"

#define NOTIFY_FLAG         (0x0001)

static thread_t *host_thread;


static void on_data(gorm_ctx_t *con, gorm_buf_t *buf)
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

void gorm_ll_host_run(void)
{
    host_thread = (thread_t *)sched_active_thread;

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
                on_data(con, buf);
            }
        }
    }
}

void gorm_ll_host_notify(gorm_ctx_t *con)
{
    (void)con;
    thread_flags_set(host_thread, NOTIFY_FLAG);
}
