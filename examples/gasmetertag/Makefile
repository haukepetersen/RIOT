APPLICATION = gasmetertag
BOARD = particle-xenon
RIOTBASE ?= $(CURDIR)/../..

# DISABLE_MODULE += auto_init
USEMODULE += core_thread_flags
# USEMODULE += stdio_rtt
USEMODULE += skald
USEMODULE += qmc5883l_int
USEMODULE += bluetil_ad

DEVELHELP ?= 0
CFLAGS += -DNDEBUG -DLOG_LEVEL=LOG_ALL

include $(RIOTBASE)/Makefile.include
