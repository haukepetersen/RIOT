APPLICATION = donglebase
BOARD ?= nrf52840dongle
RIOTBASE ?= $(CURDIR)/../..

# EXTERNAL_MODULE_DIRS += $(CURDIR)/../lib_air
# EXTERNAL_MODULE_DIRS += $(CURDIR)/../lib_hilo
# EXTERNAL_MODULE_DIRS += $(CURDIR)/../lib_xenbat

# PROGRAMMER ?= jlink

DEBUG ?= 0
ifeq (0,$(DEBUG))
  USEMODULE += stdio_null
#   CFLAGS += -DLOG_LEVEL=LOG_NONE
# else
#   PORT ?= jlink
#   USEMODULE += stdio_rtt
#   CFLAGS += -DLOG_LEVEL=LOG_ALL
endif

# USEMODULE += core_thread_flags
# USEMODULE += qmc5883l_int

# USEMODULE += lib_air
# USEMODULE += lib_hilo
# USEMODULE += lib_xenbat

USEMODULE += skald
USEMODULE += ztimer_msec
USEMODULE += ztimer_periph_rtt

USEMODULE += periph_adc

CFLAGS += -DADC_REF=SAADC_CH_CONFIG_REFSEL_Internal
CFLAGS += -DADC_GAIN=SAADC_CH_CONFIG_GAIN_Gain1_2
CFLAGS += -DADC_TACQ=SAADC_CH_CONFIG_TACQ_20us

DEVELHELP ?= 0

include $(RIOTBASE)/Makefile.include
