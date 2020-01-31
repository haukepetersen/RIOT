# name of your application
APPLICATION = sensebox_assembly
BOARD ?= sensebox_samd21
RIOTBASE ?= $(CURDIR)/../..

# modules we need
USEMODULE += fmt
USEMODULE += xtimer

USEMODULE += tsl4531x
USEMODULE += hdc1000
USEMODULE += ds18
USEMODULE += periph_adc

DEVEUI ?= 00FA3F26B4128C7D
APPEUI ?= 70B3D57ED001193D
APPKEY ?= 90579278EAA9870F1380B39E4F40FC7A

# Default radio driver is Semtech SX1276 (used by the B-L072Z-LRWAN1 board)
DRIVER ?= sx1276
 # Default region is Europe and default band is 868MHz
REGION ?= EU868
 # Include the Semtech-loramac package
USEPKG += semtech-loramac
USEMODULE += semtech_loramac_rx
USEMODULE += $(DRIVER)
 # Add the lora serialization package
USEPKG += lora-serialization
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
