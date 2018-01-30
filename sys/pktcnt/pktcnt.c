

#include <stdio.h>

#include "pktcnt.h"
#include "net/gnrc/pkt.h"
#include "net/gnrc/netif.h"

enum {
    TYPE_STARTUP,
    TYPE_PKT_TX,
    TYPE_PKT_RX,
};

typedef struct {
    char id[23];
} pktcnt_ctx_t;

static pktcnt_ctx_t ctx;

const char *keyword = "PKT";
const char *typestr[] = { "STARTUP", "PKT_TX", "PKT_RX", };

int pktcnt_init(void)
{
    /* find link layer address of lowpan device: use first device for now */
    gnrc_netif_t *dev = gnrc_netif_iter(NULL);
    if ((dev == NULL) || (dev->l2addr_len == 0)) {
        return PKTCNT_ERR_INIT;
    }
    gnrc_netif_addr_to_str(dev->l2addr, dev->l2addr_len, ctx.id);

    printf("%s %s %s\n", keyword, ctx.id, typestr[TYPE_STARTUP]);

    return PKTCNT_OK;
}

void pktcnt_signal(gnrc_pktsnip_t *pkt)
{
    (void)pkt;
}
