# name of your application
APPLICATION = nimble_gnrc

# select client or server
SERVER ?= 0
ifeq (1,$(SERVER))
  CFLAGS += -DAPP_SERVER
endif

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# So far, NimBLE only works on nRF52 based platforms
BOARD_WHITELIST := nrf52dk nrf52840dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# configure and use Nimble
USEPKG += nimble
USEMODULE += bluetil_ad
CFLAGS += -DMYNEWT_VAL_BLE_L2CAP_COC_MAX_NUM=1
CFLAGS += -DMYNEWT_VAL_BLE_MAX_CONNECTIONS=1
CFLAGS += -DMYNEWT_VAL_MSYS_1_BLOCK_COUNT=55

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1
CFLAGS += -DLOG_LEVEL=LOG_ALL
CFLAGS += -DDEBUG_ASSERT_VERBOSE

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
