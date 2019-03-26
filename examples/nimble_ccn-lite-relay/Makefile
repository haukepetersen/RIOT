APPLICATION = nimble_ccn-lite-relay

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

BOARD_WHITELIST := nrf52dk nrf52840dk ruuvitag


# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1
CFLAGS += -DLOG_LEVEL=LOG_INFO
CFLAGS += -DDEBUG_ASSERT_VERBOSE

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

USEMODULE += ps
USEMODULE += fmt
USEMODULE += shell
USEMODULE += shell_commands

# used for scanlist
USEMODULE += bleutil_ad

# configure and use Nimble
USEPKG += nimble
CFLAGS += -DMYNEWT_VAL_BLE_L2CAP_COC_MAX_NUM=1
CFLAGS += -DMYNEWT_VAL_BLE_MAX_CONNECTIONS=10

# include face management
USEMODULE += facemaster
USEMODULE += faceimp_nimble

# configure and include CCN-lite
# USEPKG += ccn-lite
CFLAGS += -DUSE_LINKLAYER
CFLAGS += -DUSE_RONR
CFLAGS += -DCCNL_UAPI_H_
CFLAGS += -DUSE_SUITE_NDNTLV
CFLAGS += -DNEEDS_PREFIX_MATCHING
CFLAGS += -DNEEDS_PACKET_CRAFTING




include $(RIOTBASE)/Makefile.include
