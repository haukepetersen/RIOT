APPLICATION = driver_sdcard_spi
include ../Makefile.tests_common

FEATURES_REQUIRED += periph_gpio
FEATURES_REQUIRED += periph_spi

USEMODULE += sdcard_spi
USEMODULE += fmt
USEMODULE += shell
USEMODULE += xtimer


# set default device parameters in case they are undefined
NUM_OF_SD_CARDS ?= 1
#default config for samr21-xpro:
SDCARD_SPI_CONFIGS ?= { { .spi_dev = SPI_1, .cs_pin = GPIO_PIN(0,14), .clk_pin = GPIO_PIN(1,23), .mosi_pin = GPIO_PIN(1,22) } }

#default config for remote-revb
#SDCARD_SPI_CONFIGS ?= { { .spi_dev = SPI_1, .cs_pin = GPIO_PIN(0,7),  .clk_pin = GPIO_PIN(2,4) , .mosi_pin = GPIO_PIN(2,5) } }

# export parameters
CFLAGS += "-DSDCARD_SPI_CONFIGS=$(SDCARD_SPI_CONFIGS)"
CFLAGS += "-DNUM_OF_SD_CARDS=$(NUM_OF_SD_CARDS)"

include $(RIOTBASE)/Makefile.include
