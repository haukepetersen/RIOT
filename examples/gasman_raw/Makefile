APPLICATION = gasman
BOARD ?= particle-xenon
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += core_thread_flags
USEMODULE += skald
USEMODULE += qmc5883l_int
USEMODULE += bluetil_ad
USEMODULE += periph_adc

# USEMODULE += stdio_none
USEMODULE += stdio_rtt

DEVELHELP ?= 0
CFLAGS += -DNDEBUG -DLOG_LEVEL=LOG_ALL

include $(RIOTBASE)/Makefile.include
