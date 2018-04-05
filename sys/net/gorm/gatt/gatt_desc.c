

#include <string.h>

#include "net/gorm/util.h"
#include "net/gorm/gatt/desc.h"


#define FMT_PDU_LEN             (7U)
#define EXT_PROP_LEN            (2U)
#define CLIENT_CONF_LEN         (2U)

size_t gorm_gatt_desc_pres_fmt_cb(const gorm_gatt_desc_t *desc,
                                  uint8_t *buf, size_t buf_len)
{
    (void)buf_len;
    assert(buf_len >= FMT_PDU_LEN);

    /* serialize the presentation format values */
    buf[0] = (uint8_t)(desc->arg >> 24);                /* format */
    buf[1] = (uint8_t)(desc->arg >> 16);                /* exponent */
    gorm_util_htoles(&buf[2], (desc->arg & 0xffff));    /* unit */
    buf[4] = 1;     /* BT SIG namespace */
    memset(&buf[5], 0, 2);
    return FMT_PDU_LEN;
}

size_t gorm_gatt_desc_ext_prop_cb(const gorm_gatt_desc_t *desc,
                                       uint8_t *buf, size_t buf_len)
{
    (void)buf_len;
    assert(buf_len >= EXT_PROP_LEN);

    gorm_util_htoles(buf, (desc->arg & 0xffff));
    return EXT_PROP_LEN;
}

size_t gorm_gatt_desc_user_desc_cb(const gorm_gatt_desc_t *desc,
                                   uint8_t *buf, size_t buf_len)
{
    const char *str = (const char *)desc->arg;
    size_t len = strlen(str);
    len = (len > buf_len) ? buf_len : len;
    memcpy(buf, str, len);
    return len;
}

size_t gorm_gatt_desc_client_conf_cb(const gorm_gatt_desc_t *desc,
                                     uint8_t *buf, size_t buf_len)
{
    (void)buf_len;
    assert(buf_len >= CLIENT_CONF_LEN);

    gorm_util_htoles(buf, (desc->arg & 0xffff));
    return CLIENT_CONF_LEN;
}
