MODULE = stm32_vectors

NO_AUTO_SRC = 1

$(info $(CPU_FAM) asdlfkjas)

SRC_FILE = $(CPU_LINE).c
SRCS = $(SRC_FILE)

# (file triggers compiler bug. see #5775)
SRC_NOLTO += $(SRC_FILE)

include $(RIOTBASE)/Makefile.base
