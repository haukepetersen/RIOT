

#include <stdint.h>

#include "net/mia.h"
#include "net/mia/ip.h"
#include "net/mia/dns.h"
#include "net/mia/udp.h"

#define DNS_POS             (MIA_APP_POS)
#define DNS_TRANSID         (DNS_POS + 0U)
#define DNS_FLAGS           (DNS_POS + 2U)
#define DNS_QS              (DNS_POS + 4U)
#define DNS_ANS_RRS         (DNS_POS + 6U)
#define DNS_AUTH_RRS        (DNS_POS + 8U)
#define DNS_ADD_RRS         (DNS_POS + 10U)
#define DNS_QUERY_POS       (DNS_POS + 12U)

#define DNS_TYPE_A          (0x0001)
#define DNS_CLASS_IN        (0x0001)


static uint16_t _flags = 0x0100;
static uint16_t _transid = 0;


// mia_bind_t mia_dns_ep = {
//     .next = NULL,
//     .cb = mia_dns_process,
//     .port = MIA_DNS_CLI_PORT,
// };

int mia_dns_request(mia_dns_req_t *req, mia_dns_cb_t cb, const char *url)
{
    size_t len = strlen(url);
    if ((len + DNS_QUERY_POS) >= MIA_BUFSIZE) {
        return MIA_ERR_OVERFLOW;
    }

    mia_lock();
    /* generate request payload */
    mia_ston(DNS_TRANSID, ++_transid);
    memcpy(mia_ptr(DNS_FLAGS), &_flags, 2);
    mia_ston(DNS_QS, 1);
    memset(mia_ptr(DNS_ANS_RRS), 0, 6);
    memcpy(mia_ptr(DNS_QUERY_POS), url, len);
    mia_ston((DNS_QUERY_POS + len), DNS_TYPE_A);
    mia_ston((DNS_QUERY_POS + len + 2), DNS_CLASS_IN);

    /* bind local port */
    mia_udp_bind_eph(&req->bind);
    req->cb = cb;

    /* send request */
    mia_ston(MIA_UDP_LEN, MIA_UDP_HDR_LEN + len + 16u);
    mia_udp_send(mia_ip_gateway, req->bind.port, MIA_DNS_SRV_PORT);
    mia_lock();

    return MIA_OK;
}

void mia_dns_process(mia_bind_t *ep)
{
    (void)ep;

    /* make sure the incoming packet is a DNS reply message */

    /* if so, read IP address and call the callback */



}
