# name of your application
APPLICATION = mia

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Define some modules that are used
USEMODULE += mia
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += xtimer


ifneq (,$(filter native,$(BOARD)))
  USEMODULE += netdev2_tap
endif
ifneq (,$(filter stm32f4discovery,$(BOARD)))
  USEMODULE += enc28j60
  CFLAGS += -DENC28J60_PARAM_SPI=SPI_1
  CFLAGS += -DENC28J60_PARAM_CS=GPIO_PIN\(PORT_B,12\)
  CFLAGS += -DENC28J60_PARAM_INT=GPIO_PIN\(PORT_B,11\)
  CFLAGS += -DENC28J60_PARAM_RESET=GPIO_PIN\(PORT_B,10\)
endif

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
