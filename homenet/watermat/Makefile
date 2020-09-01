APPLICATION = watermat
BOARD = particle-xenon
RIOTBASE ?= $(CURDIR)/../..

PROGRAMMER ?= jlink

DEBUG ?= 0
ifeq (0,$(DEBUG))
  USEMODULE += stdio_null
  CFLAGS += -DLOG_LEVEL=LOG_NONE
else
  PORT ?= jlink
  USEMODULE += stdio_rtt
  CFLAGS += -DLOG_LEVEL=LOG_ALL
endif

USEMODULE += xtimer
USEMODULE += periph_adc
USEMODULE += skald
USEMODULE += bluetil_ad

DEVELHELP ?= 0

include $(RIOTBASE)/Makefile.include
