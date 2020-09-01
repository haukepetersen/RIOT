
#include <string.h>

#include "random.h"
#include "net/skald.h"

#include "air.h"

static skald_ctx_t _adv;

void *air_init(uint8_t type, size_t airdata_len)
{
    uint8_t ad_base[] = { 0x02, 0x01, 0x06,
                          (airdata_len + 4), 0xff, 0xfe, 0xaf, type };

    uint8_t *pdu = _adv.pkt.pdu;
    _adv.pkt.len = (6 + sizeof(ad_base) + airdata_len);
    skald_generate_random_addr(pdu);
    memcpy(pdu + 6, ad_base, sizeof(ad_base));

    void *data = (void *)(pdu + 6 + sizeof(ad_base));
    memset(data, 0, sizeof(airdata_len));
    return data;
}

void air_start(void)
{
    skald_adv_start(&_adv);
}
