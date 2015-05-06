APPLICATION = wtimer_sleep_until
include ../Makefile.tests_common

CFLAGS += -DWTIMER=TIMER_0 -DWTIMER_CHAN=0

BOARD ?= native
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += wtimer

include $(RIOTBASE)/Makefile.include
