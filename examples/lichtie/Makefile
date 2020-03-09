APPLICATION = lichtie
BOARD = particle-xenon
RIOTBASE ?= $(CURDIR)/../..

PROD ?= 0
DEVELHELP ?= 0

ifeq (1,$(PROD))
  USEMODULE += stdio_none
  CFLAGS += -DNDEBUG -DLOG_LEVEL=LOG_NONE
else
  USEMODULE += stdio_rtt
  DEVELHELP = 1
  CFLAGS += -DNDEBUG -DLOG_LEVEL=LOG_ALL
endif

USEMODULE += skald
USEMODULE += bluetil_ad
CFLAGS += -DSKALD_INTERVAL=1000000

USEMODULE += periph_adc
USEMODULE += ds18b20
USEMODULE += bh1750fvi

include $(RIOTBASE)/Makefile.include
