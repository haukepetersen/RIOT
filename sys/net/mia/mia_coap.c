#include <stdio.h>


#include "net/mia/udp.h"
#include "net/mia/coap.h"

#define COAP_POS                (MIA_APP_POS)
#define COAP_VER                (COAP_POS)
#define COAP_CODE               (COAP_POS + 1U)
#define COAP_MID                (COAP_POS + 2U)
#define COAP_OPT                (COAP_POS + 4U)

/* types (with CoAP version encoded) */
#define COAP_TYPE_CON           (0x40)
#define COAP_TYPE_NON           (0x50)
#define COAP_TYPE_ACK           (0x60)
#define COAP_TYPE_RST           (0x70)

#define COAP_CODE_EMPTY         (0U)
/* request codes */
#define COAP_CODE_GET           ((0U << 5) | 1U)
#define COAP_CODE_POST          ((0U << 5) | 2U)
#define COAP_CODE_PUT           ((0U << 5) | 3U)
#define COAP_CODE_DELETE        ((0U << 5) | 4U)
/* response codes */
#define COAP_CODE_CONTENT       ((2U << 5) | 5U)
#define COAP_CODE_NOTFOUND      ((4U << 5) | 4U)

#define COAP_OPT_LOCATIONPATH   (8U  << 4)
#define COAP_OPT_URIPATH        (11U << 4)
#define COAP_OPT_CF             (12U << 4)
#define COAP_OPT_BLOCK          (23U << 4)
#define COAP_OPT_CF_TEXTPLAIN   (COAP_OPT_CF + 0)

void coap_dbg_print_type(void)
{
    switch (mia_buf[COAP_POS] & 0xf0) {
        case COAP_TYPE_CON: printf("CON(0)"); break;
        case COAP_TYPE_NON: printf("NON(1)"); break;
        case COAP_TYPE_ACK: printf("ACK(2)"); break;
        case COAP_TYPE_RST: printf("RST(3)"); break;
        default:            printf("unknwon");
    }
}

void coap_dbg_print_code(void)
{
    switch (mia_buf[COAP_CODE]) {
        case COAP_CODE_EMPTY:       printf("0.00(EMPTY) "); break;
        case COAP_CODE_GET:         printf("0.01(GET)   "); break;
        case COAP_CODE_POST:        printf("0.02(POST)  "); break;
        case COAP_CODE_PUT:         printf("0.03(PUT)   "); break;
        case COAP_CODE_DELETE:      printf("0.04(DELETE)"); break;
        default:                    printf("unknown     "); break;
    }
}

void coap_dbg_print_head(void)
{
    printf("CoAP request - Type: ");
    coap_dbg_print_type();
    printf(", Code: ");
    coap_dbg_print_code();
    puts("");
}


extern mia_coap_ep_t mia_coap_eps[];

static inline uint8_t opt_delta(uint8_t opt)
{
    return mia_buf[opt] >> 4;
}

static inline uint8_t opt_len(uint8_t opt)
{
    return mia_buf[opt] & 0x0f;
}


void mia_coap_process(void)
{
    coap_dbg_print_head();

    /* enable CoAP ping */
    if (mia_buf[COAP_POS] == COAP_TYPE_CON) {
        if (mia_buf[COAP_CODE] == COAP_CODE_GET) {
            mia_buf[COAP_POS] = COAP_TYPE_ACK;
            mia_buf[COAP_CODE] = COAP_CODE_CONTENT;
            uint16_t pos = COAP_OPT;
            mia_buf[pos++] = COAP_OPT_CF_TEXTPLAIN;
            mia_buf[pos++] = 0xff;
            memcpy(&mia_buf[pos], "Hello CoAP!", 11);
            pos += 11;

            uint16_t len = pos - COAP_POS;
            mia_ston(MIA_UDP_LEN, MIA_UDP_HDR_LEN + len);
            mia_udp_reply();
            return;
        }
    }

    /* if nothing fits, send a reset */
    mia_buf[COAP_CODE] = 0;
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
