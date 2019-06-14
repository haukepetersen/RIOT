#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

int main(void)
{
    int dev_id;
    int dd;
    int res;
    struct hci_dev_info di;
    struct hci_filter filter;
    char tmpstr[400];
    uint8_t buf[1000];

    /* open BLE/BT device */
    dev_id = hci_get_route(NULL);
    printf("dev_id: %i\n", dev_id);
    if (dev_id < 0) {
        return 1;
    }
    dd = hci_open_dev(dev_id);
    printf("dd: %i\n", dd);
    if (dd < 0) {
        return 1;
    }

    /* get some device info */
    res = hci_devinfo(dev_id, &di);
    if (res < 0) {
        printf("err: devinfo %i\n", res);
        goto end;
    }
    ba2str(&di.bdaddr, tmpstr);
    printf("own addr: %s\n", tmpstr);


    // // Set fd non-blocking
    // int on = 1;
    // if(ioctl(dd, FIONBIO, (char *)&on) < 0) {
    //     printf("ioctl: fail %s\n", strerror(errno));
    //     goto end;
    // }


    /* start scanning */
    /*
    int hci_le_set_scan_parameters(int dev_id, uint8_t type, uint16_t interval,
                    uint16_t window, uint8_t own_type,
                    uint8_t filter, int to);
    */
    res = hci_le_set_scan_parameters(dd, 0x01,
                                     htobs(0x0010), htobs(0x0010),
                                     0x00, 0x00, 1000);
    if (res < 0) {
        printf("err: le_set_scan_param failed %i\n", res);
        goto end;
    }

    res = hci_le_set_scan_enable(dd, 0x01, 0x00, 1000);
    if (res < 0) {
        printf("err: le_scan_enable %i\n", res);
        goto end;
    }

    /* configure filter */
    socklen_t olen;
    res = getsockopt(dd, SOL_HCI, HCI_FILTER, &filter, &olen);
    if (res < 0) {
        printf("err: getsockopt %s\n", strerror(errno));
        goto end;
    }
    /* create new filter from old one */
    struct hci_filter new_filter;
    hci_filter_clear(&new_filter);
    hci_filter_set_ptype(HCI_EVENT_PKT, &new_filter);
    hci_filter_set_event(EVT_LE_META_EVENT, &new_filter);
    /* apply it */
    res = setsockopt(dd, SOL_HCI, HCI_FILTER, &new_filter, sizeof(new_filter));
    if (res < 0) {
        printf("err: setsockopt: %s\n", strerror(errno));
        goto end;
    }

    printf("SCANNING\n");
    for (int i = 0; i < 10; i++) {
        int len = read(dd, buf, sizeof(buf));

        if (len <= 0) {
            printf("err: can't do anything with this data\n");
            continue;
        }

        puts("");

        // printf("RAW:");
        // for (int b = 0; b < len; b++) {
        //     printf(" 0x%02x", (int)buf[b]);
        // }
        // puts("");

        switch (buf[0]) {
            case HCI_COMMAND_PKT:
                printf("type: HCI_COMMAND_PKT\n");
                break;
            case HCI_ACLDATA_PKT:
                printf("type: HCI_ACLDATA_PKT\n");
                break;
            case HCI_SCODATA_PKT:
                printf("type: HCI_SCODATA_PKT\n");
                break;
            case HCI_EVENT_PKT:
                printf("type: HCI_EVENT_PKT\n");
                break;
            case HCI_VENDOR_PKT:
                printf("type: HCI_VENDOR_PKT\n");
                break;
        }
        if (buf[0] != HCI_EVENT_PKT) {
            printf("err: no event pkt\n");
            continue;
        }

        hci_event_hdr *evt_hdr = (hci_event_hdr *)(buf + 1);
        printf("hci_event - type: 0x%02x, plen: %i\n",
               (int)evt_hdr->evt, (int)evt_hdr->plen);
        if (evt_hdr->evt != EVT_LE_META_EVENT) {
            printf("err: no LE_META_EVENT\n");
            continue;
        }

        evt_le_meta_event *meta_evt = (evt_le_meta_event *)(buf + 1 + HCI_EVENT_HDR_SIZE);
        // printf("meta_evt - subevent: 0x%02x\n", (int)meta_evt->subevent);
        if (meta_evt->subevent != EVT_LE_ADVERTISING_REPORT) {
            printf("err: subevent is not EVT_LE_ADVERTISING_REPORT\n");
            continue;
        }

        le_advertising_info *ai = (le_advertising_info *)(meta_evt->data + 1);
        ba2str(&ai->bdaddr, tmpstr);
        printf("adv_info - type: 0x%02x, bdaddr_type: 0x%02x addr: %s, len: %i\n",
               (int)ai->evt_type, (int)ai->bdaddr_type, tmpstr, (int)ai->length);

        uint8_t *ad = (ai->data);
        size_t pos = 0;
        while (pos < (size_t)ai->length) {
            size_t len = ad[pos++];
            size_t type = ad[pos];

            // printf("AD len 0x%02x, type 0x%02x\n", (int)len, (int)type);
            if (type == 0x09) {
                strncpy(tmpstr, (char *)&ad[pos + 1], len);
                tmpstr[len - 1] = '\0';
                printf("NAME: %s\n", tmpstr);
            }

            pos += (len);
        }
    }

    /* reset filter */
    res = setsockopt(dd, SOL_HCI, HCI_FILTER, &filter, sizeof(filter));
    if (res < 0) {
        printf("err: set back old filter: %s\n", strerror(errno));
        goto end;
    }

    /* stop scanning */
    res = hci_le_set_scan_enable(dd, 0x00, 0x00, 1000);
    if (res < 0) {
        printf("err: failed to disable scanning: %s\n", strerror(errno));
    }

end:
    res = hci_close_dev(dd);
    printf("close res: %i\n", res);

    return 0;
}
