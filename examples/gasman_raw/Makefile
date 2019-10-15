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

CFLAGS += -DNRFBLE_TXPOWER_DEFAULT=RADIO_TXPOWER_TXPOWER_Pos8dBm

DEVELHELP ?= 0
CFLAGS += -DNDEBUG -DLOG_LEVEL=LOG_NONE

include $(RIOTBASE)/Makefile.include
