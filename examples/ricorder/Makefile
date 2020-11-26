APPLICATION = cwa-tracker
RIOTBASE ?= $(CURDIR)/../..
BOARD ?= nrf52dk

DEVELHELP ?= 0
QUIET ?= 1

# enables shell and STDIO for debugging
TERM ?= 1

# define dependencies
# BLE
USEPKG += nimble
USEMODULE += nimble_scanner
USEMODULE += bluetil_addr
USEMODULE += bluetil_ad
# RTC
USEMODULE += ds3231
USEMODULE += periph_gpio_irq
USEMODULE += periph_rtt
# display
USEPKG += u8g2
# storage
USEMODULE += vfs
USEMODULE += fatfs_vfs
USEMODULE += mtd_sdcard
# misc system modules
USEMODULE += fmt
USEMODULE += xtimer

# enable shell when in debugging mode
ifeq (1,$(TERM))
  USEMODULE += shell
  USEMODULE += shell_commands
  USEMODULE += ps
else
  USEMODULE += stdio_null
endif

# define pin and other board specific configuration for specific boards
ifeq (particle-xenon,$(BOARD))
  PROGRAMMER = jlink

  ifeq (1,$(TERM))
    USEMODULE += stdio_rtt
    TERMPROG ?= $(RIOTTOOLS)/jlink/jlink.sh
    TERMFLAGS ?= term-rtt
  endif

  CFLAGS += -DSDCARD_SPI_PARAM_SPI=SPI_DEV\(0\)
  CFLAGS += -DSDCARD_SPI_PARAM_CS=GPIO_PIN\(0,31\)
  CFLAGS += -DSDCARD_SPI_PARAM_CLK=GPIO_PIN\(1,15\)
  CFLAGS += -DSDCARD_SPI_PARAM_MOSI=GPIO_PIN\(1,13\)
  CFLAGS += -DSDCARD_SPI_PARAM_MISO=GPIO_PIN\(1,14\)
endif

ifeq (nrf52dk,$(BOARD))
  CFLAGS += -DSDCARD_SPI_PARAM_SPI=SPI_DEV\(0\)
  CFLAGS += -DSDCARD_SPI_PARAM_CS=GPIO_PIN\(0,22\)
  CFLAGS += -DSDCARD_SPI_PARAM_CLK=GPIO_PIN\(0,25\)
  CFLAGS += -DSDCARD_SPI_PARAM_MOSI=GPIO_PIN\(0,23\)
  CFLAGS += -DSDCARD_SPI_PARAM_MISO=GPIO_PIN\(0,24\)
endif

include $(RIOTBASE)/Makefile.include
