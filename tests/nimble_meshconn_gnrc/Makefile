BOARD ?= nrf52dk
include ../Makefile.tests_common

export PKG_SOURCE_LOCAL_NIMBLE ?= /home/hauke/dev/ble/mynewt-nimble

# include shell support
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Include GNRC and RPL
ifneq (,$(BR))
  USEMODULE += gnrc_sixlowpan_border_router_default
else
  USEMODULE += gnrc_ipv6_router_default
endif

USEMODULE += gnrc_netif_single
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_icmpv6_echo
USEMODULE += auto_init_gnrc_rpl
USEMODULE += gnrc_rpl

# Setup Nimble
USEMODULE += nimble_meshconn

# REMOVE
USEMODULE += bluetil_addr
DEVELHELP ?= 1
CFLAGS += -DDEBUG_ASSERT_VERBOSE

TEST_ON_CI_WHITELIST += nrf52dk

include $(RIOTBASE)/Makefile.include
