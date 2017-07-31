



typedef struct {
    uint16_t optnum;
    uint16_t buf_len;
    uint8_t *buf;
} hcoap_opt_t;

typedef struct {
    uint8_t *hdr;
    uint8_t *token;
    hcoap_opt_t options[HCOAP_OPTIONS_MAX];
    uint8_t *payload;
    unsigned payload_len;
} hcoap_pkt_t;


/**
 * @brief
 *
 * @param pkt [description]
 * @param optnum [description]
 *
 * @return  pointer to RAW option
 * @return  NULL if option not present
 */
int hcoap_core_find_opt(hcoap_pkt_t *pkt, hcoap_opt_t **opt, uint16_t optnum, int start);

int hcoap_parse(hcoap_pkt_t *pkt, uint8_t *buf, size_t buf_len);


static uint8_t *parse_hdr(hcoap_pkt_t *pkt, uint8_t *bufpos);
static uint8_t *parse_token(hcoap_pkt_t *pkt, uint8_t *bufpos);
static uint8_t *parse_options(hcoap_pkt_t *pkt, uint8_t *bufpos);
