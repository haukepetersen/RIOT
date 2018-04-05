MODULE = gorm_gatt

ifneq (,$(filter gorm_peripheral,$(USEMODULE)))
  SRC += gatt_server.c gatt_tab.c gatt_desc.c
endif

include $(RIOTBASE)/Makefile.base
