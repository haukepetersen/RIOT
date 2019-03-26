# name of your application
APPLICATION = nimble_gnrc_autoconn2

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# So far, NimBLE only works on nRF52 based platforms
BOARD_WHITELIST := nrf52dk nrf52840dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6_router_default
USEMODULE += gnrc_sixlowpan_router_default
USEMODULE += gnrc_udp
# Add a routing protocol
USEMODULE += gnrc_rpl
USEMODULE += auto_init_gnrc_rpl
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

USEMODULE += auto_init_gnrc_netif

# configure and use Nimble
USEMODULE += nimble_netif_autoconn_ipsp
NIMBLE_MAX_CONNECTIONS ?= 5

# all of these should be automatically defined by the autoconn_ipsp module...
USEPKG += nimble
# USEMODULE += nimble_netif
USEMODULE += nimble_scanner
USEMODULE += bluetil_ad
USEMODULE += bluetil_addr
USEMODULE += event_timeout
CFLAGS += -DMYNEWT_VAL_BLE_L2CAP_COC_MAX_NUM=1
CFLAGS += -DMYNEWT_VAL_BLE_MAX_CONNECTIONS=5
# NimBLEs internal buffer need to hold one IPv6 MTU per connection
# for the internal MTU of 100 byte, we need 13 mbufs per connection...
CFLAGS += -DMYNEWT_VAL_MSYS_1_BLOCK_COUNT=70

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1
# DEVELHELP ?= 0

# for dev
CFLAGS += -DLOG_LEVEL=LOG_INFO
CFLAGS += -DDEBUG_ASSERT_VERBOSE

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
