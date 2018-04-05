
#include "irq.h"

#include "net/gorm/pduq.h"
#include "net/gorm/pdupool.h"

static gorm_pduq_t pool = GORM_PDUQ_INIT;
static size_t fill = 0;

void gorm_pdupool_addmem(void *mem, size_t len)
{
    size_t cnt = (len / sizeof(gorm_buf_t));
    gorm_buf_t *buffers = (gorm_buf_t *)mem;

    unsigned is = irq_disable();
    for (size_t i = 0; i < cnt; i++) {
        gorm_pduq_enq(&pool, &buffers[i]);
    }
    fill += cnt;
    irq_restore(is);
}

gorm_buf_t *gorm_pdupool_get(size_t num)
{
    gorm_buf_t *buf = NULL;

    unsigned is = irq_disable();
    if (num <= fill) {
        buf = pool.head;
        for (size_t i = 0; i < num; i++) {
            pool.head = pool.head->next;
        }
        fill -= num;
    }
    irq_restore(is);

    return buf;
}

void gorm_pdupool_return(gorm_buf_t *buf)
{
    unsigned is = irq_disable();
    gorm_pduq_enq(&pool, buf);
    ++fill;
    irq_restore(is);
}
