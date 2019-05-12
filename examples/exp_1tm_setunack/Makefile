# name of your application
APPLICATION = exp_1tm_setunack

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# So far, NimBLE only works on nRF52 based platforms
BOARD_WHITELIST := nrf52dk nrf52840dk nrf52832-mdk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += fmt
USEMODULE += luid
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Include NimBLE
USEPKG += nimble
USEMODULE += nimble_mesh
# needed?
CFLAGS += -DMYNEWT_VAL_BLE_MESH_CFG_CLI=1

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1
# TMP - dev only, remove when PRing
CFLAGS += -DLOG_LEVEL=LOG_INFO
CFLAGS += -DDEBUG_ASSERT_VERBOSE
# USEMODULE += mpu_stack_guard

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
