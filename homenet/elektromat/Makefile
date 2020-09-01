APPLICATION = elektromat
BOARD = particle-xenon
RIOTBASE ?= $(CURDIR)/../..

EXTERNAL_MODULE_DIRS += $(CURDIR)/../lib_air
EXTERNAL_MODULE_DIRS += $(CURDIR)/../lib_hilo
EXTERNAL_MODULE_DIRS += $(CURDIR)/../lib_xenbat
USEMODULE += lib_air
USEMODULE += lib_hilo
USEMODULE += lib_xenbat

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
ifeq (henning,$(HAUS))
  # henning (96U/kWh)
  CFLAGS += -DPOWER_CONST=TODO
else
  # hauke (75U/kWh)
  CFLAGS += -DPOWER_CONST=37500000
endif

DEVELHELP ?= 0

include $(RIOTBASE)/Makefile.include
