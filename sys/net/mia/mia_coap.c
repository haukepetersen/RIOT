#include <stdio.h>


#include "net/mia/udp.h"
#include "net/mia/coap.h"

#define COAP_POS                (MIA_APP_POS)
#define COAP_TYPE               (COAP_POS)
#define COAP_CODE               (COAP_POS + 1U)
#define COAP_MID                (COAP_POS + 2U)
#define COAP_OPT                (COAP_POS + 4U)

#define TOK_MASK                (0x0f)

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

extern mia_coap_ep_t mia_coap_get_eps[];


mia_bind_t mia_coap_ep = {
    .next = NULL,
    .cb = mia_coap_process,
    .port = MIA_COAP_PORT
};

/**
 * @brief   Message ID used for sending NON messaged, randomly initialized
 */
static uint16_t mid = 0x1234;

static inline void set_type(uint8_t type)
{
    mia_buf[COAP_TYPE] = (type | (mia_buf[COAP_TYPE] & 0x0f));
}

static inline uint8_t get_type(void)
{
    return (mia_buf[COAP_TYPE] & 0xf0);
}

static inline uint8_t opt_delta(uint8_t opt)
{
    return mia_buf[opt] >> 4;
}

static inline uint8_t opt_len(uint8_t opt)
{
    return mia_buf[opt] & 0x0f;
}

static inline uint16_t opt_pos(void)
{
    return COAP_OPT + (mia_buf[COAP_TYPE] & TOK_MASK);
}

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

uint16_t prepare_resp(uint8_t code, uint8_t ct)
{
    uint16_t pos = opt_pos();
    /* prepare CoAP header */
    mia_buf[COAP_CODE] = code;
    /* set ct option and terminate options*/
    mia_buf[pos++] = ct;
    mia_buf[pos++] = 0xff;
    return pos;
}

uint16_t get_well_known_core(void)
{
    return 0;
}

uint16_t get_foo_bar(void)
{
    uint16_t pos = prepare_resp(COAP_CODE_CONTENT, COAP_OPT_CF_TEXTPLAIN);
    memcpy(mia_ptr(pos), "Moin moin, blubb blubb", 22);
    return pos + 22;
}

// static mia_coap_ep_t mia_coap_get_int[] = {
//     {"/.well-knwon/core", get_well_known_core},
//     {"/foo/bar", get_foo_bar},
//     {NULL, NULL}
// };

void mia_coap_process(mia_bind_t *ep)
{
    (void)ep;
    // mia_cb_t *handler;
    uint16_t pos;

    coap_dbg_print_head();

    /* we only react on CON and NON, for those we prepare the response here */
    switch (get_type()) {
        case COAP_TYPE_CON:
            set_type(COAP_TYPE_ACK);
            break;
        case COAP_TYPE_NON:
            /* we respond with a NON message, but use a new message ID */
            mia_ston(COAP_MID, mid++);
            break;
        case COAP_TYPE_ACK:
        case COAP_TYPE_RST:
        default:
            return;
    }

    /* if code is empty, respond with an empty RST (CoAP ping) */
    if (mia_buf[COAP_CODE] == COAP_CODE_EMPTY) {
        mia_buf[COAP_POS] |= COAP_TYPE_RST;
        mia_buf[COAP_CODE] = COAP_CODE_EMPTY;
        mia_udp_reply();
        return;
    }

    // handler = get_handler();
    // if (handler) {
        // pos = handler();
    // }
    pos = get_foo_bar();


    mia_ston(MIA_UDP_LEN, (pos - COAP_POS) + MIA_UDP_HDR_LEN);
    mia_udp_reply();
}
