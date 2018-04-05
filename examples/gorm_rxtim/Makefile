# name of your application
APPLICATION = gorm_rxtim

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk
BOARD_WHITELIST += nrf52dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += xtimer
USEMODULE += core_thread_flags

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
