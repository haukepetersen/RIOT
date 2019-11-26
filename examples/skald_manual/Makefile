# name of your application
APPLICATION = skald_manual

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# include Skald
USEMODULE += skald_manual
USEMODULE += random
USEMODULE += periph_rtt
USEMODULE += bluetil_ad

# USEMODULE += stdio_none

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
# CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
