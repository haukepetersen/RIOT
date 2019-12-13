

#include <stdint.h>

#define DNS_POS             (MIA_APP_POS)
#define DNS_TRANSID         (DNS_POS + 0U)
#define DNS_FLAGS           (DNS_POS + 2U)
#define DNS_QS              (DNS_POS + 4U)
#define DNS_ANS_RRS         (DNS_POS + 6U)
#define DNS_AUTH_RRS        (DNS_POS + 8U)
#define DNS_ADD_RRS         (DNS_POS + 10U)
#define DNS_QUERY_POS       (DNS_POS + 12U)


static uint16_t _flags = 0x0100;
static uint16_t _transid = 0;


mia_bind_t mia_dns_ep = {
    .next = NULL,
    .cb = mia_dns_process,
    .port = MIA_DNS_CLI_PORT,
};

void mia_dhcp_request(mia_dns_req_t *req, mia_dns_cb_t cb, const char *url)
{
    mia_lock();
    /* generate request payload */
    mia_ston(DNS_TRANSID, ++_transid);
    memcpy(mia_ptr(DNS_FLAGS), Q_FLAGS, 2);
    mit_ston(DNS_QS, 1);
    memset(mia_ptr(DNS_ANS_RRS), 0, 6);
    // set actual query
    mia_ston(MIA_UDP_LEN, MIA_UDP_HDR_LEN + len);

    /* bind local port */
    mia_udp_bind_eph(&req->ep);

    /* send request */
    mia_udp_send(mia_ip_gateway, req->ep.port, MIA_DNS_SRV_PORT);
    mia_lock();
}

void mia_dns_process(mia_bind_t *ep)
{
    (void)ep;


}
