APPLICATION = rpble_br
BOARD ?= nrf52840dongle
include $(CURDIR)/../../riot/Makefile.include

# Default to using ethos for providing the uplink when not on native
UPLINK ?= cdc-ecm

# Include packages that pull up and auto-init the link layer.
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_sixlowpan_border_router_default
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_rpl
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
# USB Modules
USEMODULE += auto_init_usbus
USEMODULE += usbus_cdc_ecm
# BLE modules
USEMODULE += nimble_netif
USEMODULE += nimble_rpble
NIMBLE_MAX_CONN ?= 10
-DNIMBLE_RPBLE_ADV_ITVL_MS=100
-DMYNEWT_VAL_BLE_LL_TX_PWR_DBM=4

# USE_DHCPV6 ?= 1
# IPV6_PREFIX ?= fd23:affe::/32

USEMODULE += gnrc_dhcpv6_client_6lbr
CFLAGS += -DCONFIG_GNRC_DHCPV6_CLIENT_6LBR_STATIC_ROUTE=1
CFLAGS += -DCONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF=3
CFLAGS += -DCONFIG_GNRC_IPV6_NIB_OFFL_NUMOF=32
CFLAGS += -DCONFIG_GNRC_IPV6_NIB_NUMOF=16

include $(RIOTBASE)/Makefile.include
