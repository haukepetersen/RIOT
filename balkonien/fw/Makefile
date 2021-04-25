# name of your application
APPLICATION = hello-world

# If no BOARD is found in the environment, use this default:
BOARD ?= particle_xenon
RIOTBASE ?= $(CURDIR)/../..

PROGRAMMER ?= jlink
RIOT_TERMINAL ?= jlink

DBG ?= 1
ifeq (1,$(DBG))
  DEVELHELP ?= 1
  USEMODULE += stdio_rtt
  CFLAGS += -DDEBUG_ASSERT_VERBOSE
  CFLAGS += -DLOG_LEVEL=LOG_INFO
else
  USEMODULE += stdio_null
  CFLAGS += -DLOG_LEVEL=LOG_NONE
endif

USEMODULE += periph_adc
USEMODULE += bh1750fvi
USEMODULE += ztimer_msec ztimer_periph_rtt

include $(RIOTBASE)/Makefile.include
