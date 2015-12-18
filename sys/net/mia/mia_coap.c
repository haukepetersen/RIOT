#include <stdio.h>


#include "net/mia/udp.h"
#include "net/mia/coap.h"



extern mia_coap_ep_t mia_coap_eps[];

#define COAP_POS            MIA_APP_POS


void mia_coap_process(void)
{
    puts("hello coap");

    mia_buf[COAP_POS] |= 0x30;
    mia_udp_reply();
}
