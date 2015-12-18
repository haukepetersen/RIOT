# name of your application
APPLICATION = mia

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Define some modules that are used
USEMODULE += mia
USEMODULE += inet_csum
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += xtimer


ifneq (,$(filter native,$(BOARD)))
  USEMODULE += netdev2_tap
  $(info BOO)
else
  USEMODULE += enc28j60
  $(info BAR)
endif

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
