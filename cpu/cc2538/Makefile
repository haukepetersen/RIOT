# Define the module that is built:
MODULE = cpu

# Add a list of subdirectories, that should also be built:
DIRS = periph $(RIOTCPU)/cortexm_common

# Include the radio, if enabled
ifneq (,$(filter cc2538,$(USEMODULE)))
  DIRS += radio
endif

include $(RIOTBASE)/Makefile.base
