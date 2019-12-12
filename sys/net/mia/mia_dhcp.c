
#include "net/ethernet.h"

#include "net/mia.h"
#include "net/mia/eth.h"
#include "net/mia/ip.h"
#include "net/mia/udp.h"
#include "net/mia/dhcp.h"

#include "net/mia/print.h"

#define ENABLE_DEBUG            (0)
#include "debug.h"

#define DHCP_HDR_LEN            (236U)  /* does not include the magic cookie */

#define DHCP_POS                (MIA_APP_POS)
#define DHCP_OP                 (DHCP_POS + 0U)
#define DHCP_HTYPE              (DHCP_POS + 1U)
#define DHCP_HLEN               (DHCP_POS + 2U)
#define DHCP_HOPS               (DHCP_POS + 3U)
#define DHCP_XID                (DHCP_POS + 4U)
#define DHCP_SECS               (DHCP_POS + 8U)
#define DHCP_FLAGS              (DHCP_POS + 10U)
#define DHCP_CIADDR             (DHCP_POS + 12U)
#define DHCP_YIADDR             (DHCP_POS + 16U)
#define DHCP_SIADDR             (DHCP_POS + 20U)
#define DHCP_GIADDR             (DHCP_POS + 24U)
#define DHCP_CHADDR             (DHCP_POS + 28U)
#define DHCP_SNAME              (DHCP_POS + 44U)
#define DHCP_FILE               (DHCP_POS + 108U)
#define DHCP_OPTIONS            (DHCP_POS + 236U)
#define DHCP_COOKIE             (DHCP_OPTIONS)
#define DHCP_FIRST_OP           (DHCP_POS + 240U)

#define OP_REQUEST              (0x01)
#define OP_REPLY                (0x02)

#define OPT_MASK                (1U)
#define OPT_ROUTER              (3U)
#define OPT_BCAST               (28U)
#define OPT_REQIP               (50U)
#define OPT_IP_LEASE_TIME       (51U)
#define OPT_TYPE                (53U)
#define OPT_SERVER_ID           (54U)
#define OPT_PARAM_REQ           (55U)
#define OPT_RENEWAL_TIME        (58U)
#define OPT_REBINDING_TIME      (59U)
#define OPT_END                 (255U)


#define OPT_TYPE_DISCOVER       (1U)
#define OPT_TYPE_OFFER          (2U)
#define OPT_TYPE_REQUEST        (3U)
#define OPT_TYPE_DECLINE        (4U)
#define OPT_TYPE_ACK            (5U)
#define OPT_TYPE_NAK            (6U)
#define OPT_TYPE_RELEASE        (7U)
#define OPT_TPYE_INFORM         (8U)


#define DIS_GAP_0               (16U)
#define DIS_GAP_1               (202U)

/**
 * @brief   Fixed head for discover packet
 *
 * | OP | HTYPE | HLEN | HOPS |
 * ----------------------------
 * |     fixed random XID     |
 * ----------------------------
 * |  SECS      | bcast flag  |
 */
static const uint8_t dis_head[] = {OP_REQUEST, 0x01, 0x06, 0x00,
                                   0xa0, 0xb0, 0xc0, 0xd0,  /* fixed XID */
                                   0x00, 0x00, 0x80, 0x00};

static const uint8_t dis_options[] = {
    0x63, 0x82, 0x53, 0x63,                 /* magic cookie (see RFC 951) */
    OPT_TYPE, 1, OPT_TYPE_DISCOVER,
    OPT_PARAM_REQ, 3, OPT_MASK, OPT_ROUTER, OPT_BCAST,
    OPT_END,
};

static const uint8_t req_options[] = {
    OPT_TYPE, 1, OPT_TYPE_REQUEST,
    OPT_REQIP, 4, 0, 0, 0, 0,
    OPT_SERVER_ID, 4, 0, 0, 0, 0,
    OPT_IP_LEASE_TIME, 4, 0, 0, 0xa8, 0xc0,
    OPT_END,
};

mia_bind_t mia_dhcp_ep = {
    .next = NULL,
    .cb = mia_dhcp_process,
    .port = DHCP_CLI_PORT
};

static inline uint8_t opt_code(int opt)
{
    return mia_buf[opt];
}

static inline uint8_t opt_len(int opt)
{
    return mia_buf[opt + 1];
}

static inline uint8_t opt_val(int opt)
{
    return mia_buf[opt + 2];
}

static inline uint8_t *opt_ptr(int opt)
{
    return mia_ptr(opt + 2);
}

static inline int opt_next(int opt)
{
    return opt + mia_buf[opt + 1] + 2;
}

void mia_dhcp_request(void)
{
    mia_lock();
    memcpy(mia_ptr(DHCP_POS), dis_head, sizeof(dis_head));
    memset(mia_ptr(DHCP_CIADDR), 0, DIS_GAP_0);
    memcpy(mia_ptr(DHCP_CHADDR), mia_mac, ETHERNET_ADDR_LEN);
    memset(mia_ptr(DHCP_CHADDR + ETHERNET_ADDR_LEN), 0, DIS_GAP_1);
    memcpy(mia_ptr(DHCP_OPTIONS), dis_options, sizeof(dis_options));

    mia_ston(MIA_UDP_LEN, MIA_UDP_HDR_LEN + DHCP_HDR_LEN + sizeof(dis_options));
    mia_udp_send((uint8_t *)mia_bcast, DHCP_CLI_PORT, DHCP_SRV_PORT);
    mia_unlock();
}

void mia_dhcp_process(mia_bind_t *ep)
{
    (void)ep;
    int opt = DHCP_FIRST_OP;

    /* we expect the DHCP message type as first option! */
    if (opt_code(opt) != OPT_TYPE) {
        DEBUG("[mia] dhcp: first option was not the type, no good\n");
        return;
    }

    if (opt_val(opt) == OPT_TYPE_OFFER) {
        /* we reply with a request */
        DEBUG("[mia] dhcp: got offer, sending out request\n");

        mia_buf[DHCP_POS] = OP_REQUEST;
        memcpy(mia_ptr(DHCP_FIRST_OP), req_options, sizeof(req_options));
        memcpy(mia_ptr(DHCP_FIRST_OP + 5), mia_ptr(DHCP_YIADDR), 4);
        memcpy(mia_ptr(DHCP_FIRST_OP + 11), mia_ptr(DHCP_SIADDR), 4);
        memset(mia_ptr(DHCP_YIADDR), 0, 4);

        mia_ston(MIA_UDP_LEN,
                 MIA_UDP_HDR_LEN + DHCP_HDR_LEN + 4 + sizeof(req_options));
        memcpy(mia_ptr(MIA_IP_SRC), mia_bcast, 4);
        mia_udp_reply();
    }
    else if (opt_val(opt) == OPT_TYPE_ACK) {
        DEBUG("[mia] dhcp: got ACK, setting address to what was given\n");
        opt = opt_next(opt);
        /* set IP address that got acked */
        memcpy(mia_ip_addr, mia_ptr(DHCP_YIADDR), 4);
        /* filter for bcast, mask and gateway */
        while (opt_code(opt) != OPT_END) {
            if (opt_code(opt) == OPT_ROUTER) {
                memcpy(mia_ip_gateway, opt_ptr(opt), 4);
            }
            else if (opt_code(opt) == OPT_MASK) {
                mia_ip_mask = 0;
                for (int i = 0; i < 4; i++) {
                    if (opt_ptr(opt)[i] == 0xff) {
                        ++mia_ip_mask;
                    }
                }
            }
            else if (opt_code(opt) == OPT_BCAST) {
                memcpy(mia_ip_bcast, opt_ptr(opt), 4);
            }
            opt = opt_next(opt);
        }

#if ENABLE_DEBUG
        DEBUG("[mia] dhcp: binding addr:");
        mia_print_ip_addr(mia_ip_addr);
        DEBUG(", bcast:");
        mia_print_ip_addr(mia_ip_bcast);
        DEBUG(", mask:%i, gateway:", (int)mia_ip_mask);
        mia_print_ip_addr(mia_ip_gateway);
        DEBUG("\n");
#endif
    }
    else {
        DEBUG("[mia] dhcp: response option type not handable for us.\n");
    }
}
