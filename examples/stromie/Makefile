APPLICATION = stromie
BOARD = particle-xenon
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += stdio_rtt
USEMODULE += periph_adc
USEMODULE += xtimer

USEMODULE += skald
USEMODULE += bluetil_ad

CFLAGS += -DSKALD_INTERVAL=1000000

DEVELHELP ?= 0
CFLAGS += -DNDEBUG -DLOG_LEVEL=LOG_NONE

include $(RIOTBASE)/Makefile.include
