

#include "ccnl_app.h"
// #include "tlsf-malloc.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/pktdump.h"

#include "ccn-lite-riot.h"



/* 10kB buffer for the heap should be enough for everyone */
#define TLSF_BUFFER     (10240 / sizeof(uint32_t))
// static uint32_t _tlsf_heap[TLSF_BUFFER];

static gnrc_netreg_entry_t _dump;

void ccnl_app_init(void)
{
    // tlsf_add_global_pool(_tlsf_heap, sizeof(_tlsf_heap));

    ccnl_core_init();

    ccnl_start();

    /* get the default interface */
    gnrc_netif_t *netif;

    /* set the relay's PID, configure the interface to use CCN nettype */
    if (((netif = gnrc_netif_iter(NULL)) == NULL) ||
        (ccnl_open_netif(netif->pid, GNRC_NETTYPE_CCN) < 0)) {
        puts("Error registering at network interface!");
        return;
    }

    printf("context for GNRC_NETTYPE_CCN is %i\n", (int)GNRC_NETTYPE_CCN);
    printf("context for GNRC_NETTYPE_CCN_CHUNK is %i\n", (int)GNRC_NETTYPE_CCN_CHUNK);
    printf("GNRC_NETTYPE_NUMOF numof: %u\n", (unsigned)GNRC_NETTYPE_NUMOF);

    printf("registered nettype %u with ", GNRC_NETTYPE_CCN_CHUNK);

    gnrc_netreg_entry_init_pid(&_dump, GNRC_NETREG_DEMUX_CTX_ALL, gnrc_pktdump_pid);
    gnrc_netreg_register(GNRC_NETTYPE_CCN_CHUNK, &_dump);

    int num = gnrc_netreg_num(GNRC_NETTYPE_CCN_CHUNK, GNRC_NETREG_DEMUX_CTX_ALL);
    printf("found %i entries for CHUNK\n", num);
}
