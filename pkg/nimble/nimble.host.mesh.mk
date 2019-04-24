MODULE = nimble_mesh

# xxx: the following files do not compile for RIOT, so we skip them
IGNORE := testing.c

SRC := $(filter-out $(IGNORE),$(wildcard *.c))
include $(RIOTBASE)/Makefile.base
