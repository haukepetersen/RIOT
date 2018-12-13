# name of your application
APPLICATION = sensebox_assembly

# If no BOARD is found in the environment, use this default:
BOARD ?= sensebox_samd21

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

BOARD_INSUFFICIENT_MEMORY := arduino-duemilanove arduino-uno

# we want to use SAUL:
#USEMODULE += saul_default
# include the shell:
USEMODULE += shell
USEMODULE += shell_commands
# additional modules for debugging:
USEMODULE += ps
#USEMODULE += xtimer
USEMODULE += tsl4531x
USEMODULE += hdc1000
USEMODULE += ds18

FEATURES_REQUIRED = periph_adc

DEVEUI ?= 00FA3F26B4128C7D
APPEUI ?= 70B3D57ED001193D
APPKEY ?= 90579278EAA9870F1380B39E4F40FC7A

# Default radio driver is Semtech SX1276 (used by the B-L072Z-LRWAN1 board)
DRIVER ?= sx1276
 # Default region is Europe and default band is 868MHz
REGION ?= EU868
 # Include the Semtech-loramac package
USEPKG += semtech-loramac
USEMODULE += $(DRIVER)
USEMODULE += fmt
 # Add the lora serialization package
USEPKG += lora-serialization
FEATURES_REQUIRED += periph_rtc
CFLAGS += -DREGION_$(REGION)
CFLAGS += -DDEVEUI=\"$(DEVEUI)\" -DAPPEUI=\"$(APPEUI)\" -DAPPKEY=\"$(APPKEY)\"
CFLAGS += -DLORAMAC_ACTIVE_REGION=LORAMAC_REGION_$(REGION)
CFLAGS += -DLORAMAC_DEFAULT_TX_MODE=LORAMAC_TX_UNCNF

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
