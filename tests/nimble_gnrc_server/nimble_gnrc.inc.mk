# Set the tests default configuration
APP_PORT ?= 23517
APP_ADDR ?= \"affe::1\"
APP_NODENAME ?= \"nimble_l2cap_test_server\"

# Apply configuration values
CFLAGS += -DAPP_PORT=$(APP_PORT)
CFLAGS += -DAPP_ADDR=$(APP_ADDR)
CFLAGS += -DAPP_NODENAME=$(APP_NODENAME)

# Configure GNRC
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_router_default
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_sixlowpan_router_default
USEMODULE += gnrc_udp
USEMODULE += gnrc_sock_udp
# We use RPL in this setup, although it is only single hop...
USEMODULE += auto_init_gnrc_rpl
USEMODULE += gnrc_rpl

# configure NimBLE
USEPKG += nimble
USEMODULE += nimble_netif
CFLAGS += -DMYNEWT_VAL_BLE_L2CAP_COC_MAX_NUM=1
CFLAGS += -DMYNEWT_VAL_BLE_L2CAP_COC_MTU=250
CFLAGS += -DMYNEWT_VAL_BLE_MAX_CONNECTIONS=1
CFLAGS += -DMYNEWT_VAL_MSYS_1_BLOCK_COUNT=55

INCLUDES += -I$(CURDIR)/../nimble_gnrc_server/include
