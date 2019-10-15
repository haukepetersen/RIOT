APPLICATION = gasman
BOARD ?= particle-xenon
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

USEMODULE += core_thread_flags
USEMODULE += periph_adc
USEMODULE += qmc5883l_int
USEMODULE += skald
USEMODULE += bluetil_ad

DEVELHELP ?= 0

include $(RIOTBASE)/Makefile.include
