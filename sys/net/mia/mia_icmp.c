
#include "xtimer.h"
#include "net/protnum.h"
#include "net/inet_csum.h"

#include "net/mia.h"
#include "net/mia/ip.h"
#include "net/mia/icmp.h"

#define ENABLE_DEBUG            (1)
#include "debug.h"



#define ICMP_TYPE_ECHO_REQ      (8U)
#define ICMP_TYPE_ECHO_RPLY     (0U)
#define ICMP_ECHO_ID            (0xdeed)


static mia_cb_t ping_cb = NULL;
static uint16_t ping_seq = 0;


void mia_icmp_process(void)
{
    /* we only react to echo requests */
    if (mia_buf[MIA_ICMP_TYPE] == ICMP_TYPE_ECHO_REQ) {
        /* set type to reply and re-calculate checksum */
        mia_buf[MIA_ICMP_TYPE] = ICMP_TYPE_ECHO_RPLY;
        mia_ston(MIA_ICMP_CSUM, 0);
        mia_ston(MIA_ICMP_CSUM, ~inet_csum(0, mia_ptr(MIA_ICMP_POS),
                                       mia_ntos(MIA_IP_LEN) - MIA_IP_HDR_LEN));
        mia_ip_reply(mia_ntos(MIA_IP_LEN) - MIA_IP_HDR_LEN);
    }
    else if (mia_buf[MIA_ICMP_TYPE] == ICMP_TYPE_ECHO_RPLY) {
        if (ping_cb) {
            ping_cb();
        }
    }
    else {
        DEBUG("[mia] icmp: received unknown ICMP message (type %i)\n",
              (int)mia_buf[MIA_ICMP_TYPE]);
    }
}

int mia_icmp_ping(uint8_t *addr, mia_cb_t cb)
{
    int res = 0;
    uint32_t now = xtimer_now();

    mia_lock();

    ping_cb = cb;
    mia_buf[MIA_ICMP_TYPE] = ICMP_TYPE_ECHO_REQ;
    mia_buf[MIA_ICMP_CODE] = 0;
    memset(mia_ptr(MIA_ICMP_CSUM), 0, 4);   /* also sets the identifier to 0 */
    mia_ston(MIA_ICMP_ECHO_SEQ, ping_seq++);
    memcpy(mia_ptr(MIA_ICMP_ECHO_DATA), &now, 4);
    mia_ston(MIA_ICMP_CSUM,
         ~inet_csum(0, mia_ptr(MIA_ICMP_POS), MIA_ICMP_HDR_LEN + 4));
    /* send out the packet */
    res = mia_ip_send(addr,  PROTNUM_ICMP, MIA_ICMP_HDR_LEN + 4);

    mia_unlock();
    return res;
}

void mia_icmp_unreachable(uint8_t code)
{
    (void)code;
    mia_buf[MIA_IP_PROTO] = PROTNUM_ICMP;
    mia_ip_reply(mia_ntos(MIA_IP_LEN));
}
