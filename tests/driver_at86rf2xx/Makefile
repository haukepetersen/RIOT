APPLICATION = driver_at86rf2xx
include ../Makefile.tests_common

FEATURES_REQUIRED = periph_spi periph_gpio

BOARD_INSUFFICIENT_RAM := stm32f0discovery

ifneq (,$(filter iot-lab_M3,$(BOARD)))
  USEMODULE += ng_at86rf231
  export ATRF_SPI ?= SPI_0
  export ATRF_CS ?= GPIO_11
  export ATRF_INT ?= GPIO_12
  export ATRF_RESET ?= GPIO_13
  export ATRF_SLEEP ?= GPIO_14
endif
ifneq (,$(filter samr21-xpro,$(BOARD)))
  USEMODULE += ng_at86rf233
  export ATRF_SPI ?= SPI_0
  export ATRF_CS ?= GPIO_4
  export ATRF_INT ?= GPIO_5
  export ATRF_RESET ?= GPIO_6
  export ATRF_SLEEP ?= GPIO_7
  export ATRF_SPI_SPEED ?= SPI_SPEED_1MHZ
endif

USEMODULE += ng_at86rf2xx         # default to the generic driver
USEMODULE += ng_netbase
USEMODULE += ng_nomac
USEMODULE += ng_pktdump
USEMODULE += uart0
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

CFLAGS += -DDEVELHELP

ifneq (,$(ATRF_SPI))
  CFLAGS += -DATRF_SPI=$(ATRF_SPI)
else
  CFLAGS += -DATRF_SPI=SPI_0        # set default
endif
ifneq (,$(ATRF_CS))
  CFLAGS += -DATRF_CS=$(ATRF_CS)
else
  CFLAGS += -DATRF_CS=GPIO_0        # set default
endif
ifneq (,$(ATRF_INT))
  CFLAGS += -DATRF_INT=$(ATRF_INT)
else
  CFLAGS += -DATRF_INT=GPIO_1        # set default
endif
ifneq (,$(ATRF_SLEEP))
  CFLAGS += -DATRF_SLEEP=$(ATRF_SLEEP)
else
  CFLAGS += -DATRF_SLEEP=GPIO_2        # set default
endif
ifneq (,$(ATRF_RESET))
  CFLAGS += -DATRF_RESET=$(ATRF_RESET)
else
  CFLAGS += -DATRF_RESET=GPIO_3        # set default
endif
ifneq (,$(ATRF_SPI_SPEED))
  CFLAGS += -DATRF_SPI_SPEED=$(ATRF_SPI_SPEED)
endif

include $(RIOTBASE)/Makefile.include
