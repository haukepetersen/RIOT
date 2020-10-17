include ../Makefile.tests_common

# Blacklist iotlab boards since a different device has the same i2c address
BOARD_BLACKLIST := iotlab-a8-m3 \
                   iotlab-m3

USEMODULE += ds3231
USEMODULE += xtimer
USEMODULE += shell

ifeq (particle-xenon,$(BOARD))
  $(info Moin xenon)
  PROGRAMMER = jlink
  USEMODULE += stdio_rtt
  TERMPROG ?= $(RIOTTOOLS)/jlink/jlink.sh
  TERMFLAGS ?= term-rtt

  CFLAGS += -DSDCARD_SPI_PARAM_SPI=SPI_DEV\(0\)
  CFLAGS += -DSDCARD_SPI_PARAM_CS=GPIO_PIN\(0,31\)
  CFLAGS += -DSDCARD_SPI_PARAM_CLK=GPIO_PIN\(1,15\)
  CFLAGS += -DSDCARD_SPI_PARAM_MOSI=GPIO_PIN\(1,13\)
  CFLAGS += -DSDCARD_SPI_PARAM_MISO=GPIO_PIN\(1,14\)

#   CFLAGS +=
endif

include $(RIOTBASE)/Makefile.include
