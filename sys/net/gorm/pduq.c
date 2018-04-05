

#include "irq.h"
#include "assert.h"

#include "net/gorm/pduq.h"

void gorm_pduq_enq(gorm_pduq_t *queue, gorm_buf_t *buf)
{
    assert(queue && buf);

    unsigned is = irq_disable();
    /* if list is non-empty: let last element point to buf */
    if (queue->tail) {
        queue->tail->next = buf;
    }
    /* special case: list was empty, so head also points to buf */
    else {
        queue->head = buf;
    }

    buf->next = NULL;
    queue->tail = buf;
    irq_restore(is);
}

gorm_buf_t *gorm_pduq_deq(gorm_pduq_t *queue)
{
    assert(queue);

    unsigned is = irq_disable();
    gorm_buf_t *buf = queue->head;
    /* increment next counter only if list is non-empty */
    if (buf) {
        queue->head = buf->next;
    }
    /* if list is now empty, also reset tail */
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    irq_restore(is);

    return buf;
}
