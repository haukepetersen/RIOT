APPLICATION = xtimer_sleep_until
include ../Makefile.tests_common

CFLAGS += -DXTIMER=TIMER_0 -DXTIMER_CHAN=0

BOARD ?= native
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += xtimer

include $(RIOTBASE)/Makefile.include
