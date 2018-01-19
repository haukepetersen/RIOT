# name of your application
APPLICATION = ruuvi_coap_server

# If no BOARD is found in the environment, use this default:
BOARD ?= ruuvitag

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# pull SAUL sensors
USEMODULE += saul_default

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules
USEMODULE += gnrc_ipv6_default
USEMODULE += gcoap
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo

# Required by gcoap example
USEMODULE += od
USEMODULE += fmt
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1
CFLAGS += -DDEBUG_ASSERT_VERBOSE
CFLAGS += -DGCOAP_PDU_BUF_SIZE=512
CFLAGS += -DGCOAP_STACK_SIZE=4096


include $(RIOTBASE)/Makefile.include
