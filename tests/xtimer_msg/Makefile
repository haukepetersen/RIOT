APPLICATION = xtimer_msg
include ../Makefile.tests_common

BOARD_INSUFFICIENT_RAM := stm32f0discovery

CFLAGS += -DXTIMER=TIMER_0 -DXTIMER_CHAN=0
USEMODULE += xtimer

include $(RIOTBASE)/Makefile.include
