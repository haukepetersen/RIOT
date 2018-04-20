# name of your application
APPLICATION = gorm_gatt

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# configure Gorm
USEMODULE += gorm_gatt
USEMODULE += auto_init_gorm

# for demonstration purposes we include statistics for Gorm
USEMODULE += gorm_stats

# include some pre-defined services
USEMODULE += ggs_riot_led
USEMODULE += ggs_ips

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP
CFLAGS += -DDEBUG_ASSERT_VERBOSE

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
