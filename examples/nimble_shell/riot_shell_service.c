

#include <stdint.h>
#include <stdio.h>

// #include "nimble/nimble_port.h"
// #include "host/ble_hs.h"
// #include "host/util/util.h"

#include "os/os_mbuf.h"


#include "host/ble_gatt.h"
#include "host/ble_uuid.h"

#include "riot_shell_service.h"

#define UUID_SHELL_SVC      0x54, 0x4f, 0x49, 0x52, 0x00, 0x00, 0x72, 0x65, \
                            0x70, 0x75, 0x73, 0x00, 0x01, 0x00, 0x00, 0xf0

#define UUID_SHELL_CHAR     0x54, 0x4f, 0x49, 0x52, 0x00, 0x00, 0x72, 0x65, \
                            0x70, 0x75, 0x73, 0x00, 0x02, 0x00, 0x00, 0xf0


static const ble_uuid128_t _uuid_shell_svc = BLE_UUID128_INIT(UUID_SHELL_SVC);
static const ble_uuid128_t _uuid_shell_char = BLE_UUID128_INIT(UUID_SHELL_CHAR);


// static char _buf[128];

static int _shell_handle(uint16_t conn_handle, uint16_t attr_handle,
                         struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    printf("access to con 0x%04x , attr 0x%04x\n", (int)conn_handle, (int)attr_handle);
    (void)ctxt;
    (void)arg;

    switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            printf("Read OP\n");
            const char foo[] = "Hello, this is some ugly shit";
            os_mbuf_append(ctxt->om, foo, sizeof(foo));
            break;
        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            printf("Write OP\n");
            break;
        default:
            printf("unkown operation %i\n", (int)ctxt->op);
            break;
    }



    return 0;
}

static const struct ble_gatt_svc_def _shell_service[] = {
    {
        /* define the RIOT shell service */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = (ble_uuid_t *)&_uuid_shell_svc.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            /* the shell service has only a single shell char */
            {
                .uuid = (ble_uuid_t *)&_uuid_shell_char.u,
                .access_cb = _shell_handle,
                .flags = (BLE_GATT_CHR_F_READ |
                          BLE_GATT_CHR_F_WRITE |
                          BLE_GATT_CHR_F_NOTIFY),
            },
            {
                0, /* end of chars */
            },
        },
    },
    {
        0, /* end of services */
    },
};


void riot_shell_service_init(void)
{
    puts("Hello shell service");

    if (ble_gatts_count_cfg(_shell_service) != 0) {
        puts("ERROR counting");
        return;
    }
    if (ble_gatts_add_svcs(_shell_service) != 0) {
        puts("ERROR adding service");
        return;
    }
}
