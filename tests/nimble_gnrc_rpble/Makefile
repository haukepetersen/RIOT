# name of your application
APPLICATION = nimble_rpble

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
USEMODULE += auto_init_gnrc_netif
# Add a routing protocol
USEMODULE += gnrc_rpl
USEMODULE += auto_init_gnrc_rpl
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# configure and use Nimble
USEPKG += nimble
USEMODULE += nimble_netif_rpble
# the number of concurrent connections that can be handled
NIMBLE_MAX_CONN := 3

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1
CFLAGS += -DLOG_LEVEL=LOG_INFO
CFLAGS += -DDEBUG_ASSERT_VERBOSE

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
