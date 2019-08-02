# name of your application
APPLICATION = nimble_mesh

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# So far, NimBLE only works on selected nRF5x based platforms
BOARD_WHITELIST := nrf52dk nrf52840dk nrf52832-mdk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Helper modules
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += periph_gpio_irq

# Include NimBLE and its used submodules
USEPKG += nimble
USEMODULE += nimble_mesh
USEMODULE += nimble_svc_gatt
USEMODULE += nimble_svc_gap


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

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# NAME ?= foobert
# CFLAGS += -DMYNEWT_VAL_BLE_MESH_DEVICE_NAME=\"$(NAME)\"

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# REMOVE - dev only, remove when PRing
CFLAGS += -DLOG_LEVEL=LOG_INFO
CFLAGS += -DDEBUG_ASSERT_VERBOSE

include $(RIOTBASE)/Makefile.include
