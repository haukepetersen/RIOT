


#include "nimble_scanner.h"
#include "nimble_riot.h"
#include "host/ble_gap.h"



#define ENABLE_DEBUG        (0)
#include "debug.h"

static nimble_scanner_cb _disc_cb = NULL;
static void *_disc_arg = NULL;
static struct ble_gap_disc_params _scan_params = { 0 };

static int _on_scan_evt(struct ble_gap_event *event, void *arg)
{
    /* only interested in the DISC event */
    if (event->type == BLE_GAP_EVENT_DISC) {
        _disc_cb(&event->disc.addr, event->disc.rssi,
                 event->disc.data, (size_t)event->disc.length_data);
    }
    else {
        DEBUG("[scanner] unknown event %i\n", (int)event->type);
    }

    return 0;
}

int nimble_scanner_start(void)
{
    DEBUG("[scanner] START\n");
    if (ble_gap_disc_active() == 0) {
        int res = ble_gap_disc(nimble_riot_own_addr_type, BLE_HS_FOREVER,
                               &_scan_params, _on_scan_evt, NULL);
        if (res != 0) {
            DEBUG("       [] ERROR: start failed (%i)\n", res);
            return NIMBLE_SCANNER_ERR;
        }
    }
    else {
        DEBUG("       [] ERROR: scanner already active\n");
    }

    return NIMBLE_SCANNER_OK;
}

void nimble_scanner_stop(void)
{
    DEBUG("[scanner] STOP\n");
    if (ble_gap_disc_active() == 1) {
        int res = ble_gap_disc_cancel();
        if (res == 0) {
            DEBUG("       [] ok\n");
        }
        else {
            DEBUG("       [] failed (%i)\n", res);
        }
    }
    else {
        DEBUG("       [] skipping - not scanning\n");
    }
}

int nimble_scanner_status(void)
{
    return (ble_gap_disc_active()) ?
                            NIMBLE_SCANNER_SCANNING : NIMBLE_SCANNER_STOPPED;
}

int nimble_scanner_init(const struct ble_gap_disc_params *params,
                        nimble_scanner_cb disc_cb, void *arg)
{
    assert(disc_cb);

    memcpy(&_scan_params, params, sizeof(_scan_params));

    _disc_cb = disc_cb;
    _disc_arg = arg;

    DEBUG("[scanner] init: parameters all set\n");

    return NIMBLE_SCANNER_OK;
}
