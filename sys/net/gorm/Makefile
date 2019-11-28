MODULE = gorm

SRC = ll.c ll_trx.c util.c gorm.c buf.c addr.c

ifneq (,$(filter gorm_peripheral,$(USEMODULE)))
  SRC += ll_chan.c ll_ctrl.c l2cap.c
endif

ifneq (,$(filter gorm_gatt,$(USEMODULE)))
  SRC += uuid.c
  DIRS += $(RIOTBASE)/sys/net/gorm/gatt
endif

ifneq (,$(filter gorm_gap,$(USEMODULE)))
  DIRS += $(RIOTBASE)/sys/net/gorm/gap
endif

ifneq (,$(filter gorm_l2cap_coc,$(USEMODULE)))
  SRC += coc.c
endif

ifneq (,$(filter gorm_arch_riot,$(USEMODULE)))
  DIRS += $(RIOTBASE)/sys/net/gorm/arch/riot
endif

ifneq (,$(filter gorm_netif,$(USEMODULE)))
  DIRS += $(RIOTBASE)/sys/net/gorm/netif
endif

# include select build-in services
ifneq (,$(filter gorm_service_%,$(USEMODULE)))
  DIRS += $(RIOTBASE)/sys/net/gorm/service
endif

include $(RIOTBASE)/Makefile.base
