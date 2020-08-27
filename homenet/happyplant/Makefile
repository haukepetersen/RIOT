APPLICATION = happyplant
BOARD ?= airfy-beacon
RIOTBASE ?= $(CURDIR)/../..

DEBUG ?= 1
ifeq (0,$(DEBUG))
  USEMODULE += stdio_null
  CFLAGS += -DLOG_LEVEL=LOG_NONE
else
  CFLAGS += -DLOG_LEVEL=LOG_ALL
endif

USEMODULE += core_thread_flags
USEMODULE += periph_adc
USEMODULE += bh1750fvi

USEMODULE += skald
USEMODULE += bluetil_ad

DEVELHELP ?= 0

include $(RIOTBASE)/Makefile.include
