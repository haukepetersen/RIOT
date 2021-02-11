APPLICATION = gasman
BOARD ?= particle-xenon
RIOTBASE ?= $(CURDIR)/../..

EXTERNAL_MODULE_DIRS += $(CURDIR)/../lib_air
EXTERNAL_MODULE_DIRS += $(CURDIR)/../lib_hilo
EXTERNAL_MODULE_DIRS += $(CURDIR)/../lib_xenbat

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
USEMODULE += qmc5883l_int
USEMODULE += ztimer_msec
USEMODULE += ztimer_periph_rtt

USEMODULE += lib_air
USEMODULE += lib_hilo
USEMODULE += lib_xenbat

DEVELHELP ?= 0

include $(RIOTBASE)/Makefile.include
