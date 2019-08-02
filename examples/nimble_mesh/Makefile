# name of your application
APPLICATION = nimble_mesh

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# So far, NimBLE only works on nRF52 based platforms
BOARD_WHITELIST := nrf52dk nrf52840dk nrf52832-mdk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Include NimBLE
USEPKG += nimble
USEMODULE += nimble_mesh


CFLAGS += -DMYNEWT_VAL_BLE_MESH=1
CFLAGS += -DMYNEWT_VAL_BLE_MESH_RELAY=1
CLFAGS += -DMYNEWT_VAL_BLE_MESH_CFG_CLI=0
CFLAGS += -DMYNEWT_VAL_BLE_MESH_HEALTH_CLI=0

CFLAGS += -DMYNEWT_VAL_BLE_MESH_PROV=1
CFLAGS += -DMYNEWT_VAL_BLE_MESH_PB_ADV=1
CFLAGS += -DMYNEWT_VAL_BLE_MESH_PB_GATT=1

CFLAGS += -DMYNEWT_VAL_BLE_MESH_FRIEND=0
CFLAGS += -DMYNEWT_VAL_BLE_MESH_LOW_POWER=0
CFLAGS += -DMYNEWT_VAL_BLE_MESH_PROXY=1
CFLAGS += -DMYNEWT_VAL_BLE_MESH_GATT_PROXY=1

CFLAGS += -DMYNEWT_VAL_BLE_MESH_ADV_BUF_COUNT=160
CFLAGS += -DMYNEWT_VAL_BLE_MESH_TX_SEG_MAX=32

# NAME ?= foobert
# CFLAGS += -DMYNEWT_VAL_BLE_MESH_DEVICE_NAME=\"$(NAME)\"

USEMODULE += periph_gpio_irq

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# TMP - dev only, remove when PRing
CFLAGS += -DLOG_LEVEL=LOG_INFO
CFLAGS += -DDEBUG_ASSERT_VERBOSE

include $(RIOTBASE)/Makefile.include
