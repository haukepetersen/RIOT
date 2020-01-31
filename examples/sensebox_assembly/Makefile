# name of your application
APPLICATION = sensebox_assembly
BOARD ?= sensebox_samd21
RIOTBASE ?= $(CURDIR)/../..

# modules we need
USEMODULE += fmt
USEMODULE += xtimer
# sensors
USEMODULE += tsl4531x
USEMODULE += hdc1000
USEMODULE += ds18
USEMODULE += periph_adc
# lora
USEMODULE += sx1276
USEPKG += lora-serialization
USEPKG += semtech-loramac
USEMODULE += semtech_loramac_rx

ifeq (fu,$(ID))
  DEVEUI ?= 00FA3F26B4128C7D
  APPEUI ?= 70B3D57ED001193D
  APPKEY ?= 90579278EAA9870F1380B39E4F40FC7A
endif
include $(CURDIR)/nodecfg.inc.mk
ifeq (,$(DEVEUI))
  $(error ERROR: lora key configuration specified, define a ID env var!)
endif

$(info Building for device '$(ID)')
$(info -> DEVEUI:$(DEVEUI), APPEUI:$(APPEUI), APPKEY:$(APPKEY))

# set keys
CFLAGS += -DDEVEUI=\"$(DEVEUI)\"
CFLAGS += -DAPPEUI=\"$(APPEUI)\"
CFLAGS += -DAPPKEY=\"$(APPKEY)\"

# configure LoRa
 # Add the lora serialization package
CFLAGS += -DREGION_EU868
CFLAGS += -DLORAMAC_ACTIVE_REGION=LORAMAC_REGION_EU868
CFLAGS += -DLORAMAC_DEFAULT_TX_MODE=LORAMAC_TX_UNCNF

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
