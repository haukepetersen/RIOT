APPLICATION = wtimer_msg
include ../Makefile.tests_common

BOARD_INSUFFICIENT_RAM := stm32f0discovery

CFLAGS += -DWTIMER=TIMER_0 -DWTIMER_CHAN=0
USEMODULE += wtimer

include $(RIOTBASE)/Makefile.include
