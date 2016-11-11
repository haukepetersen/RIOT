APPLICATION = driver_sdcard_spi
include ../Makefile.tests_common

FEATURES_REQUIRED += periph_gpio
FEATURES_REQUIRED += periph_spi

USEMODULE += sdcard_spi
USEMODULE += fmt
USEMODULE += shell


# set default device parameters in case they are undefined
TEST_SDCARD_SPI  ?= SPI_1
TEST_SDCARD_CS   ?= GPIO_PIN\(0,14\)

# export parameters
CFLAGS += -DTEST_SDCARD_SPI=$(TEST_SDCARD_SPI)
CFLAGS += -DTEST_SDCARD_CS=$(TEST_SDCARD_CS)

include $(RIOTBASE)/Makefile.include