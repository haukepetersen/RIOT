#include <stdio.h>


#include "net/mia/udp.h"
#include "net/mia/coap.h"

#define COAP_POS                (MIA_APP_POS)
#define COAP_VER                (COAP_POS)
#define COAP_CODE               (COAP_POS + 1U)
#define COAP_MID                (COAP_POS + 2U)
#define COAP_OPT                (COAP_POS + 4U)


#define COAP_OPT_URIPATH        (11U)
#define COAP_OPT_BLOCK          (23U)

extern mia_coap_ep_t mia_coap_eps[];

static inline uint8_t opt_delta(opt)
{
    return mia_buf[opt] >> 4;
}

static inline uint8_t opt_len(opt)
{
    return mia_buf[opt] & 0x0f;
}


void mia_coap_process(void)
{
    puts("hello coap");

    /* enable CoAP ping */
    mia_buf[COAP_POS] |= 0x30;
    mia_udp_reply();

    // uint32_t req_hash = 0;

    // uint8_t opt = COAP_OPT;

    /* hash requested ep */

    // mit_coap_ep_t *ep = mia_coap_eps;
    // while (ep) {
    //     if (req_hash == ep->hash) {

    //     }

    //     ep++;
    // }
}
