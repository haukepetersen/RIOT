#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define ATT_CID  0x0123

int main() {
    printf("init\n");
    bdaddr_t src_addr, dst_addr;
    struct sockaddr_l2 l2cap_address;

    // Get the destination address
    str2ba("e1:a3:1e:75:29:b0", &dst_addr);
    socklen_t opt = sizeof(dst_addr);

    // Get host address
    int hci_device_id = hci_get_route(NULL);
    int hci_socket = hci_open_dev(hci_device_id);
    hci_devba(hci_device_id, &src_addr);

    /* create L2CAP socket, and bind it to the local adapter */
    printf("Creating socket...\n");
    int l2cap_socket = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if(l2cap_socket < 0) {
        printf("Error creating socket\n");
        return -1;
    }

    memset(&l2cap_address, 0, sizeof(l2cap_address));
    l2cap_address.l2_family = AF_BLUETOOTH;
    l2cap_address.l2_bdaddr = src_addr;
    l2cap_address.l2_cid = htobs(ATT_CID);

    printf("Binding socket...\n");
    bind(l2cap_socket, (struct sockaddr*)&l2cap_address, sizeof(l2cap_address));
    listen(l2cap_socket, 1);

    // Connect to bluetooth peripheral
    printf("Creating connection... ");
    uint16_t hci_handle;
    int result = hci_le_create_conn(hci_socket,
        htobs(0x0004), htobs(0x0004), 0,
        LE_RANDOM_ADDRESS, dst_addr, LE_PUBLIC_ADDRESS,
        htobs(0x0006) /*min_interval*/, htobs(0x0020) /*max_interval*/,
        htobs(0) /*latency*/, htobs(200) /*supervision_timeout*/,
        htobs(0x0001), htobs(0x0001), &hci_handle, 25000);
    printf("%d\n", result);

    fd_set afds;
    struct timeval tv = {
        .tv_usec = 3000
    };
    // int len;
    uint8_t buffer[1024];
    printf("Entering loop\n");
    while (1) {
        printf("Selecting...\n");
        /* now select and accept() client connections. */
        select(l2cap_socket + 1, &afds, NULL, NULL, &tv);

        printf("Accepting...\n");
        bdaddr_t addr;
        // len = (int)sizeof(addr);
        int client_socket = accept(l2cap_socket, (struct sockaddr *)&addr, &opt);

        printf("Reading...\n");
        /* you can now read() what the client sends you */
        int ret = read(client_socket, buffer, sizeof(buffer));
        printf("data len: %d\n", ret);
        for (int i = 0; i < ret; i++) {
        printf("%02x", ((int)buffer[i]) & 0xff);
        }
        printf("\n");
        close(client_socket);
    }

    return 0;
}
